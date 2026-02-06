#include <agrb/utils/exec.hpp>

namespace agrb
{
    vk::Result single_time_exec::end()
    {
        vk::Fence fence;
        fence_pool.request(&fence, 1);
        vk_device.resetFences(fence, loader);
        command_buffer.end(loader);
        vk::SubmitInfo submit_info;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        queue.vk_queue.submit(submit_info, fence, loader);
        auto res = vk_device.waitForFences(fence, true, UINT64_MAX, loader);
        queue.pool.primary.release(command_buffer);
        fence_pool.release(fence);
        return res;
    }
} // namespace agrb