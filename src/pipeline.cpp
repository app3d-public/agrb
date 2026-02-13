#include <acul/io/fs/file.hpp>
#include <agrb/pipeline.hpp>

namespace agrb
{
    graphics_config &graphics_config::load_defaults()
    {
        input_assembly_info.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(false);
        viewport_info.setViewportCount(1).setScissorCount(1);
        rasterization_info.setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthBiasEnable(false);
        color_blend_attachment
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | // Red
                               vk::ColorComponentFlagBits::eG | // Green
                               vk::ColorComponentFlagBits::eB | // Blue
                               vk::ColorComponentFlagBits::eA)  // Alpha
            .setBlendEnable(false);
        color_blend_info.setLogicOpEnable(false).setAttachmentCount(1).setPAttachments(&color_blend_attachment);
        depth_stencil_info.setDepthTestEnable(true)
            .setDepthWriteEnable(true)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(false);
        dynamic_state_enables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        dynamic_state_info.setPDynamicStates(dynamic_state_enables.data())
            .setDynamicStateCount(dynamic_state_enables.size())
            .setFlags(vk::PipelineDynamicStateCreateFlags());
        return *this;
    }

    graphics_config &graphics_config::enable_alpha_blending()
    {
        color_blend_attachment.setBlendEnable(true)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);
        return *this;
    }

    bool shader_module::load(device &device)
    {
        vk::ShaderModuleCreateInfo create_info;
        create_info.setCodeSize(data->code.size()).setPCode(reinterpret_cast<const u32 *>(data->code.data()));
        auto res = device.vk_device.createShaderModule(&create_info, nullptr, &module, device.loader);
        if (res != vk::Result::eSuccess) return false;
        return true;
    }

    void prepare_base_graphics_pipeline(pipeline_batch<vk::GraphicsPipelineCreateInfo>::artifact &artifact,
                                        vk::ShaderModule *shaders, device &device)
    {
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(shaders[0])
            .setPName("main");
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(shaders[1])
            .setPName("main");
        artifact.config.viewport_info.setViewportCount(1).setPViewports(nullptr).setScissorCount(1).setPScissors(
            nullptr);
        artifact.create_info.setStageCount(2)
            .setPStages(artifact.config.shader_stages.data())
            .setPVertexInputState(&artifact.config.vertex_input_info)
            .setPInputAssemblyState(&artifact.config.input_assembly_info)
            .setPViewportState(&artifact.config.viewport_info)
            .setPRasterizationState(&artifact.config.rasterization_info)
            .setPMultisampleState(&artifact.config.multisample_info)
            .setPColorBlendState(&artifact.config.color_blend_info)
            .setPDepthStencilState(&artifact.config.depth_stencil_info)
            .setPDynamicState(&artifact.config.dynamic_state_info)
            .setLayout(artifact.config.pipeline_layout)
            .setRenderPass(artifact.config.render_pass)
            .setSubpass(artifact.config.subpass)
            .setBasePipelineIndex(-1)
            .setBasePipelineHandle(nullptr);
    }

    APPLIB_API void
    configure_compute_pipeline_artifact(pipeline_batch<vk::ComputePipelineCreateInfo>::artifact &artifact,
                                        vk::PipelineLayout &layout, agrb::device &device,
                                        const vk::ShaderModule &shader)
    {
        artifact.config.pipeline_layout = layout;
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(shader)
            .setPName("main");
        artifact.create_info.setStage(artifact.config.shader_stages.back())
            .setBasePipelineIndex(-1)
            .setBasePipelineHandle(nullptr)
            .setLayout(artifact.config.pipeline_layout);
    }

    static void append_shader_node_to_cache(const umbf::Library::Node &node, shader_cache &cache)
    {
        if (node.is_folder)
        {
            for (const auto &child : node.children) append_shader_node_to_cache(child, cache);
            return;
        }
        auto &asset = node.asset;
        if (asset.header.vendor_sign != AGRB_VENDOR_ID || asset.header.type_sign != AGRB_TYPE_ID_SHADER) return;

        for (const auto &block : asset.blocks)
        {
            if (!block) continue;
            const u32 sign = block->signature();
            if (sign != AGRB_SIGN_ID_SHADER) continue;
            auto shader = acul::static_pointer_cast<agrb::shader_block>(block);
            auto &dst = cache[shader->id];
            dst.data = shader;
        }
    }

    APPLIB_API acul::op_result load_shader_library(const acul::path &library_path, shader_cache &cache)
    {
        acul::shared_ptr<umbf::File> file;
        ACUL_TRY(umbf::File::read_from_disk(library_path.str(), file));
        if (!file) return acul::make_op_error(ACUL_OP_NULLPTR);
        if (file->header.type_sign != umbf::sign_block::format::library || file->blocks.empty())
            return acul::make_op_error(ACUL_OP_ERROR_GENERIC);

        auto root = file->blocks.front();
        if (!root || root->signature() != umbf::sign_block::library) return acul::make_op_error(ACUL_OP_ERROR_GENERIC);

        auto library = acul::static_pointer_cast<umbf::Library>(root);
        append_shader_node_to_cache(library->file_tree, cache);
        return acul::make_op_success();
    }

    APPLIB_API acul::op_result get_shader(u64 id, vk::ShaderModule &out, shader_cache &cache, device &device,
                                          const acul::path &library_path)
    {
        auto it = cache.find(id);
        if (it == cache.end())
        {
            ACUL_TRY(load_shader_library(library_path, cache));
            it = cache.find(id);
            if (it == cache.end()) return {AGRB_OP_ID_NOT_FOUND, AGRB_OP_DOMAIN};
        }
        if (!it->second.data) return acul::make_op_error(ACUL_OP_NULLPTR);

        auto &shader = it->second;
        if (!shader.module && !shader.load(device)) return {AGRB_OP_GPU_RESOURCE_FAILED, AGRB_OP_DOMAIN};
        out = shader.module;
        return acul::make_op_success();
    }

    namespace streams
    {
        void write_shader(acul::bin_stream &stream, umbf::Block *block)
        {
            auto *shader = (shader_block *)block;
            u64 code_size = static_cast<u64>(shader->code.size());
            stream.write(shader->id).write<u64>(code_size).write(shader->code.data(), shader->code.size());
        }

        umbf::Block *read_shader(acul::bin_stream &stream)
        {
            shader_block *block = acul::alloc<shader_block>();
            u64 code_size = 0;
            stream.read(block->id).read(code_size);
            block->code.resize(code_size);
            stream.read(block->code.data(), code_size);
            return block;
        }
    } // namespace streams
} // namespace agrb
