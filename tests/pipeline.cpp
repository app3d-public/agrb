#include <agrb/pipeline.hpp>
#include "env.hpp"

using namespace agrb;

void test_pipeline()
{
    init_library();
    Enviroment env;
    init_environment(env);

    graphics_pipeline_batch batch;
    batch.shaders.resize(2);
    auto &vert_shader = batch.shaders[0];
    auto &frag_shader = batch.shaders[1];

    const char *data_dir = getenv("TEST_DATA_DIR");
    assert(data_dir);
    acul::path p = data_dir;
    vert_shader.path = p / "vs_test.spv";
    frag_shader.path = p / "fs_test.spv";

    assert(vert_shader.load(env.d).success());
    assert(frag_shader.load(env.d).success());

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
    prepare_base_graphics_pipeline(artifact, batch.shaders[0], batch.shaders[1], env.d);
    assert(batch.allocate_pipelines(env.d, 1));

    env.d.vk_device.destroyRenderPass(render_pass, nullptr, env.d.loader);
    env.d.vk_device.destroyPipelineLayout(pipeline_layout, nullptr, env.d.loader);
    destroy_library();
}
