#pragma once

#include "../device.hpp"

namespace agrb
{
    struct single_time_exec
    {
        vk::CommandBuffer command_buffer;
        vk::Device &vk_device;
        queue_family_info &queue;
        resource_pool<vk::Fence, fence_pool_alloc> &fence_pool;
        vk::DispatchLoaderDynamic &loader;

        single_time_exec(device &device)
            : vk_device(device.vk_device),
              queue(device.rd->queues.graphics),
              fence_pool(device.rd->fence_pool),
              loader(device.loader)
        {
            queue.pool.primary.request(&command_buffer, 1);
            vk::CommandBufferBeginInfo begin_info;
            begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            command_buffer.begin(begin_info, loader);
        }

        APPLIB_API vk::Result end();
    };

} // namespace agrb