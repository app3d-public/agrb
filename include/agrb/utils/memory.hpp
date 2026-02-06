#pragma once

#include <acul/scalars.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace agrb
{
    inline size_t get_alignment(size_t size, size_t min_offset)
    {
        return min_offset > 0 ? (size + min_offset - 1) & ~(min_offset - 1) : size;
    }

    inline vk::MemoryPropertyFlags get_allocation_memory_flags(VmaAllocator allocator, VmaAllocation allocation)
    {
        VmaAllocationInfo alloc_info{};
        vmaGetAllocationInfo(allocator, allocation, &alloc_info);
        VkMemoryPropertyFlags actual = 0;
        vmaGetMemoryTypeProperties(allocator, alloc_info.memoryType, &actual);
        return static_cast<vk::MemoryPropertyFlags>(actual);
    }

    inline vk::MemoryPropertyFlags missing_memory_flags(VmaAllocator allocator, VmaAllocation allocation,
                                                        vk::MemoryPropertyFlags flags)
    {
        return flags & ~get_allocation_memory_flags(allocator, allocation);
    }

    inline VmaAllocationCreateInfo make_alloc_info(VmaMemoryUsage usage, vk::MemoryPropertyFlags required_flags,
                                                   vk::MemoryPropertyFlags preferred_flags = {}, f32 priority = 0.5f)
    {
        return {.usage = usage,
                .requiredFlags = static_cast<VkMemoryPropertyFlags>(required_flags),
                .preferredFlags = static_cast<VkMemoryPropertyFlags>(preferred_flags),
                .priority = priority};
    }
} // namespace agrb