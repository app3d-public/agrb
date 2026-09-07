#include <agrb/pipeline.hpp>
#include <acul/hash/hashmap.hpp>
#include "env.hpp"

using namespace agrb;

using shader_block_cache = acul::hashmap<u64, acul::shared_ptr<shader_block>>;

static shader_block_cache load_shader_cache(const acul::string &library_path)
{
    umbf::registry::HashResolver resolver;
    resolver.block_streams.emplace(static_cast<u32>(AGRB_TYPE_ID_SHADER), &agrb::streams::shader);
    resolver.block_streams.emplace(static_cast<u32>(AGRB_SIGN_ID_SHADER), &agrb::streams::shader);
    umbf::insert_default_segment_codecs(resolver);
    auto *prev_resolver = umbf::registry::resolver;
    umbf::registry::resolver = &resolver;

    umbf::ReadDescriptor asset;
    auto load_res = umbf::create_read_descriptor(library_path, asset);

    assert(load_res.success());
    assert(asset.file);
    assert(asset.file->type_sign == umbf::sign_block::format::raw);

    shader_block_cache cache;
    for (auto block = asset.begin(); block != asset.end(); ++block)
    {
        if (block->signature != AGRB_SIGN_ID_SHADER) continue;
        auto value = umbf::get_block(block);
        auto shader = value ? acul::make_shared<shader_block>(
                                  std::move(*static_cast<shader_block *>(value.get())))
                            : nullptr;
        if (shader) cache.emplace(shader->id, shader);
    }
    umbf::registry::resolver = prev_resolver;
    assert(!cache.empty());
    return cache;
}

void test_pipeline()
{
    init_library();
    Enviroment env;
    init_environment(env);

    graphics_pipeline_batch batch;
    shader_module vert_shader;
    shader_module frag_shader;

    const char *data_dir = getenv("TEST_DATA_DIR");
    assert(data_dir);
    acul::path p = data_dir;
    auto cache = load_shader_cache((p / "test_shaders.umlib").str());
    const u64 vs_id = 0x063E992A01000000ULL;
    const u64 fs_id = 0x063E992A02000000ULL;
    auto vs_it = cache.find(vs_id);
    auto fs_it = cache.find(fs_id);
    assert(vs_it != cache.end());
    assert(fs_it != cache.end());
    vert_shader.data = vs_it->second;
    frag_shader.data = fs_it->second;

    assert(vert_shader.load(env.d));
    assert(frag_shader.load(env.d));

    batch.artifacts.emplace_back();
    auto &artifact = batch.artifacts.back();
    artifact.config.load_defaults().enable_alpha_blending().enable_msaa(vk::SampleCountFlagBits::e1);

    // Prepare requirements for pipeline creation
    vk::PipelineLayoutCreateInfo layoutInfo{};
    vk::PipelineLayout pipeline_layout = env.d.vk_device.createPipelineLayout(layoutInfo, nullptr, env.d.loader);
    artifact.config.pipeline_layout = pipeline_layout;

    vk::AttachmentDescription attachment{};
    attachment.format = vk::Format::eR8G8B8A8Unorm;
    attachment.samples = vk::SampleCountFlagBits::e1;
    attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    attachment.initialLayout = vk::ImageLayout::eUndefined;
    attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    vk::RenderPassCreateInfo rpInfo{};
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &attachment;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    vk::RenderPass render_pass = env.d.vk_device.createRenderPass(rpInfo, nullptr, env.d.loader);

    artifact.config.render_pass = render_pass;
    artifact.config.shader_stages.emplace_back();
    artifact.config.shader_stages.back()
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vert_shader.module)
        .setPName("main");

    artifact.config.shader_stages.emplace_back();
    artifact.config.shader_stages.back()
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(frag_shader.module)
        .setPName("main");
    artifact.config.vertex_input_info.setVertexBindingDescriptionCount(artifact.config.binding_descriptions.size())
        .setVertexAttributeDescriptionCount(artifact.config.attribute_descriptions.size())
        .setPVertexBindingDescriptions(artifact.config.binding_descriptions.data())
        .setPVertexAttributeDescriptions(artifact.config.attribute_descriptions.data());
    artifact.config.viewport_info.setViewportCount(1).setPViewports(nullptr).setScissorCount(1).setPScissors(nullptr);
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

    artifact.commit = [&](vk::Pipeline pipeline) {
        assert(pipeline);
        env.d.vk_device.destroyPipeline(pipeline, nullptr, env.d.loader);
    };
    vk::ShaderModule shader_stages[2] = {vert_shader.module, frag_shader.module};
    prepare_base_graphics_pipeline(artifact, shader_stages, env.d);
    assert(batch.allocate_pipelines(env.d, 1));

    env.d.vk_device.destroyRenderPass(render_pass, nullptr, env.d.loader);
    env.d.vk_device.destroyPipelineLayout(pipeline_layout, nullptr, env.d.loader);
    destroy_library();
}
