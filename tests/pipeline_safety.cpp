#include <agrb/pipeline.hpp>
#include <cassert>
#include <cstdint>
#include <limits>

namespace
{
    u32 calls = 0, destroyed = 0, committed = 0;
    bool fail = false;
    VKAPI_ATTR VkResult VKAPI_CALL create_graphics(VkDevice, VkPipelineCache, uint32_t count,
                                                  const VkGraphicsPipelineCreateInfo *, const VkAllocationCallbacks *,
                                                  VkPipeline *pipelines)
    {
        ++calls;
        for (u32 i = 0; i < count; ++i)
            pipelines[i] = fail && i != 0 ? VK_NULL_HANDLE : reinterpret_cast<VkPipeline>(uintptr_t(0x100 + i));
        return fail ? VK_ERROR_INITIALIZATION_FAILED : VK_SUCCESS;
    }
    VKAPI_ATTR void VKAPI_CALL destroy_pipeline(VkDevice, VkPipeline pipeline, const VkAllocationCallbacks *)
    {
        assert(pipeline);
        ++destroyed;
    }
}

void test_pipeline_safety()
{
    agrb::init_library();
    {
        agrb::device device;
        device.vk_device = reinterpret_cast<VkDevice>(uintptr_t(1));
        const auto old_create = device.loader.vkCreateGraphicsPipelines;
        const auto old_destroy = device.loader.vkDestroyPipeline;
        device.loader.vkCreateGraphicsPipelines = &create_graphics;
        device.loader.vkDestroyPipeline = &destroy_pipeline;
        agrb::graphics_pipeline_batch batch;
        assert(batch.allocate_pipelines(device, 0));
        assert(!batch.allocate_pipelines(device, 1));
        batch.artifacts.emplace_back();
        batch.artifacts.emplace_back();
        // Exact failure from the user dump: five requested records, two list nodes.
        assert(!batch.allocate_pipelines(device, 5));
        assert(!batch.allocate_pipelines(device, std::numeric_limits<size_t>::max()));
        assert(!batch.allocate_pipelines(device, 2)); // Failed/unconfigured artifacts never reach Vulkan.
        assert(calls == 0);
        vk::PipelineShaderStageCreateInfo stage;
        stage.setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(reinterpret_cast<VkShaderModule>(uintptr_t(2))).setPName("main");
        for (auto &artifact : batch.artifacts)
        {
            artifact.create_info.setStageCount(1).setPStages(&stage);
            artifact.commit = [](vk::Pipeline pipeline) { assert(pipeline); ++committed; };
        }
        assert(batch.allocate_pipelines(device, 2));
        assert(calls == 1 && committed == 2 && destroyed == 0);
        fail = true;
        assert(!batch.allocate_pipelines(device, 2));
        assert(calls == 2 && committed == 2 && destroyed == 1);
        agrb::compute_pipeline_batch compute;
        compute.artifacts.emplace_back();
        assert(!compute.allocate_pipelines(device, 2));
        assert(!compute.allocate_pipelines(device, 1));
        device.loader.vkCreateGraphicsPipelines = old_create;
        device.loader.vkDestroyPipeline = old_destroy;
        device.vk_device = nullptr;
    }
    agrb::destroy_library();
}
#ifndef PROCESS_UNITTEST
int main() { test_pipeline_safety(); }
#endif
