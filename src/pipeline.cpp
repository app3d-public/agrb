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

    acul::op_result shader_module::load(device &device)
    {
        if (!acul::fs::read_binary(path, code)) return acul::make_op_error(ACUL_OP_READ_ERROR);
        vk::ShaderModuleCreateInfo create_info;
        create_info.setCodeSize(code.size()).setPCode(reinterpret_cast<const u32 *>(code.data()));
        auto res = device.vk_device.createShaderModule(&create_info, nullptr, &module, device.loader);
        if (res != vk::Result::eSuccess) return acul::make_op_error(ACUL_OP_ERROR_GENERIC);
        return acul::make_op_success();
    }

    acul::op_result prepare_base_graphics_pipeline(pipeline_batch<vk::GraphicsPipelineCreateInfo>::artifact &artifact,
                                                   shader_module &vert, shader_module &frag, device &device)
    {
        ACUL_TRY(vert.load(device));
        ACUL_TRY(frag.load(device));
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vert.module)
            .setPName("main");
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(frag.module)
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
        return acul::make_op_success();
    }

    APPLIB_API acul::op_result
    configure_compute_pipeline_artifact(pipeline_batch<vk::ComputePipelineCreateInfo>::artifact &artifact,
                                        agrb::shader_list &shaders, vk::PipelineLayout &layout, agrb::device &device,
                                        const acul::path &shader_path)
    {
        artifact.config.pipeline_layout = layout;
        shaders.emplace_back(shader_path);
        auto &comp = shaders.back();
        ACUL_TRY(comp.load(device));
        artifact.config.shader_stages.emplace_back();
        artifact.config.shader_stages.back()
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(comp.module)
            .setPName("main");
        artifact.create_info.setStage(artifact.config.shader_stages.back())
            .setBasePipelineIndex(-1)
            .setBasePipelineHandle(nullptr)
            .setLayout(artifact.config.pipeline_layout);
        return acul::make_op_success();
    }
} // namespace agrb
