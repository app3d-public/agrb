#include <agrb/utils/buffer.hpp>

#define MEM_DEDICATTED_ALLOC_MIN 536870912u

namespace agrb
{
    bool create_buffer(vk::DeviceSize size, vk::BufferUsageFlags vk_usage, vk::Buffer &buffer,
                       VmaAllocationCreateInfo alloc_info, VmaAllocation &allocation, const device &device)
    {
        vk::BufferCreateInfo buffer_info;
        buffer_info.setSize(size).setUsage(vk_usage).setSharingMode(vk::SharingMode::eExclusive);
        if (size > MEM_DEDICATTED_ALLOC_MIN) alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        return vmaCreateBuffer(device.allocator, reinterpret_cast<const VkBufferCreateInfo *>(&buffer_info),
                               &alloc_info, reinterpret_cast<VkBuffer *>(&buffer), &allocation, nullptr) == VK_SUCCESS;
    }

    bool copy_data_to_gpu_buffer_host_visible(const gpu_upload_info &upload_info, VmaAllocator &allocator,
                                              vk::MemoryPropertyFlags mem_flags)
    {
        void *dst = upload_info.data;
        if (!dst)
            if (vmaMapMemory(allocator, upload_info.allocation, &dst) != VK_SUCCESS) return false;
        memcpy(dst, upload_info.data, upload_info.size);

        if (mem_flags & vk::MemoryPropertyFlagBits::eHostCoherent)
        {
            if (vmaFlushAllocation(allocator, upload_info.allocation, 0, VK_WHOLE_SIZE) != VK_SUCCESS)
            {
                if (!upload_info.data) vmaUnmapMemory(allocator, upload_info.allocation);
                return false;
            }
        }

        if (!upload_info.data) vmaUnmapMemory(allocator, upload_info.allocation);
        return true;
    }

    bool copy_data_to_gpu_buffer_staging(const gpu_upload_info &upload_info, device &device)
    {
        buffer staging;
        staging.instance_count = 1;
        auto st_alloc_info = make_alloc_info(VMA_MEMORY_USAGE_CPU_ONLY, vk::MemoryPropertyFlagBits::eHostVisible,
                                             vk::MemoryPropertyFlagBits::eHostCoherent, 0.1f);
        construct_buffer(staging, upload_info.size);
        if (!allocate_buffer(staging, st_alloc_info, vk::BufferUsageFlagBits::eTransferSrc, device)) return false;
        if (!map_buffer(staging, device))
        {
            destroy_buffer(staging, device);
            return false;
        }
        write_to_buffer(staging, upload_info.data);
        unmap_buffer(staging, device);
        single_time_exec exec{device};
        if (upload_info.on_copy_staging) upload_info.on_copy_staging(exec, staging);
        if (upload_info.on_upload) upload_info.on_upload(exec, true);
        bool is_success = exec.end() == vk::Result::eSuccess;
        destroy_buffer(staging, device);
        return is_success;
    }

    bool copy_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device)
    {
        if (!upload_info.valid()) return false;
        auto mem_flags = get_allocation_memory_flags(device.allocator, upload_info.allocation);
        if (mem_flags & vk::MemoryPropertyFlagBits::eHostVisible)
        {
            if (!copy_data_to_gpu_buffer_host_visible(upload_info, device.allocator, mem_flags)) return false;
            if (upload_info.on_upload)
            {
                single_time_exec exec{device};
                upload_info.on_upload(exec, false);
                return exec.end() == vk::Result::eSuccess;
            }
            return true;
        }
        return copy_data_to_gpu_buffer_staging(upload_info, device);
    }

    bool move_data_to_gpu_buffer_host_visible(const gpu_upload_info &upload_info, VmaAllocator &allocator,
                                              vk::MemoryPropertyFlags mem_flags)
    {
        void *dst = upload_info.data;
        if (!dst)
        {
            if (vmaMapMemory(allocator, upload_info.allocation, &dst) != VK_SUCCESS) return false;
        }

        memmove(dst, upload_info.data, upload_info.size);

        if (mem_flags & vk::MemoryPropertyFlagBits::eHostCoherent)
        {
            if (vmaFlushAllocation(allocator, upload_info.allocation, 0, VK_WHOLE_SIZE) != VK_SUCCESS)
            {
                if (!upload_info.data) vmaUnmapMemory(allocator, upload_info.allocation);
                return false;
            }
        }

        if (!upload_info.data) vmaUnmapMemory(allocator, upload_info.allocation);
        return true;
    }

    bool move_data_to_gpu_buffer_staging(const gpu_upload_info &upload_info, device &device)
    {
        buffer staging;
        staging.instance_count = 1;
        auto st_alloc_info = make_alloc_info(VMA_MEMORY_USAGE_CPU_ONLY, vk::MemoryPropertyFlagBits::eHostVisible,
                                             vk::MemoryPropertyFlagBits::eHostCoherent, 0.1f);

        construct_buffer(staging, upload_info.size);
        if (!allocate_buffer(staging, st_alloc_info, vk::BufferUsageFlagBits::eTransferSrc, device)) return false;

        if (!map_buffer(staging, device))
        {
            destroy_buffer(staging, device);
            return false;
        }

        move_to_buffer(staging, upload_info.data);
        unmap_buffer(staging, device);

        single_time_exec exec{device};

        if (upload_info.on_copy_staging) upload_info.on_copy_staging(exec, staging);
        if (upload_info.on_upload) upload_info.on_upload(exec, true);

        bool is_success = (exec.end() == vk::Result::eSuccess);
        destroy_buffer(staging, device);
        return is_success;
    }

    bool move_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device)
    {
        if (!upload_info.valid()) return false;

        auto mem_flags = get_allocation_memory_flags(device.allocator, upload_info.allocation);
        if (mem_flags & vk::MemoryPropertyFlagBits::eHostVisible)
        {
            if (!move_data_to_gpu_buffer_host_visible(upload_info, device.allocator, mem_flags)) return false;

            if (upload_info.on_upload)
            {
                single_time_exec exec{device};
                upload_info.on_upload(exec, false);
                return exec.end() == vk::Result::eSuccess;
            }
            return true;
        }

        return move_data_to_gpu_buffer_staging(upload_info, device);
    }
} // namespace agrb
