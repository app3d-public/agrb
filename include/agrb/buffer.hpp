#pragma once
#include "device.hpp"

namespace agrb
{
    struct buffer
    {
        u32 instance_count = 0;
        void *mapped = nullptr;
        vk::Buffer vk_buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        vk::DeviceSize alignment_size = 0;
        vk::DeviceSize buffer_size = 0;
    };

    struct managed_buffer final : buffer
    {
        vk::MemoryPropertyFlags required_flags;
        vk::MemoryPropertyFlags prefered_flags;
        vk::BufferUsageFlags buffer_usage;
        VmaMemoryUsage vma_usage;
        f32 priority = 0.5f;
    };
} // namespace agrb