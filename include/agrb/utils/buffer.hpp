#pragma once

#include <acul/disposal_queue.hpp>
#include "../buffer.hpp"
#include "exec.hpp"
#include "memory.hpp"

namespace agrb
{
    inline void construct_buffer(buffer &buffer, size_t instance_size, size_t min_offset_alignment = 1)
    {
        buffer.alignment_size = get_alignment(instance_size, min_offset_alignment);
        buffer.buffer_size = buffer.alignment_size * buffer.instance_count;
    }

    inline void construct_ubo_buffer(buffer &buffer, size_t instance_size, device_runtime_data *rd)
    {
        buffer.alignment_size = rd->get_aligned_ubo_size(instance_size);
        buffer.buffer_size = buffer.alignment_size * buffer.instance_count;
    }

    inline bool map_buffer(buffer &buffer, device &device)
    {
        return vmaMapMemory(device.allocator, buffer.allocation, &buffer.mapped) == VK_SUCCESS;
    }

    inline void unmap_buffer(buffer &buffer, device &device)
    {
        if (!buffer.mapped) return;
        vmaUnmapMemory(device.allocator, buffer.allocation);
        buffer.mapped = nullptr;
    }

    inline void destroy_buffer(buffer &buffer, device &device)
    {
        unmap_buffer(buffer, device);
        vmaDestroyBuffer(device.allocator, buffer.vk_buffer, buffer.allocation);
        buffer = {};
    }

    /// @brief Create a buffer with the given size, usage flags, and memory usage
    /// @param size Size of the buffer in bytes
    /// @param vk_usage Usage flags for the Vulkan buffer
    /// @param buffer Destination Vulkan buffer
    /// @param alloc_info Allocation info
    /// @param allocation: Allocation for the buffer
    /// @param device Device
    /// @return True on success, false on failure
    APPLIB_API bool create_buffer(vk::DeviceSize size, vk::BufferUsageFlags vk_usage, vk::Buffer &buffer,
                                  VmaAllocationCreateInfo alloc_info, VmaAllocation &allocation, const device &device);

    inline bool allocate_buffer(buffer &buffer, VmaAllocationCreateInfo alloc_info, vk::BufferUsageFlags usage_flags,
                                device &device)
    {
        assert(buffer.buffer_size > 0);
        return create_buffer(buffer.buffer_size, usage_flags, buffer.vk_buffer, alloc_info, buffer.allocation, device);
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    inline void write_to_buffer(buffer &buffer, void *data, vk::DeviceSize size = VK_WHOLE_SIZE,
                                vk::DeviceSize offset = 0)
    {
        assert(buffer.mapped);
        if (size == VK_WHOLE_SIZE)
            memcpy(buffer.mapped, data, buffer.buffer_size);
        else
        {
            char *mem_offset = static_cast<char *>(buffer.mapped);
            mem_offset += offset;
            memcpy(mem_offset, data, size);
        }
    }

    /**
     * Moves the specified data to the mapped buffer
     * @param data Pointer to the data to move
     * @param size Size of the data to move
     * @param offset Byte offset from beginning of mapped region
     *
     **/
    inline void move_to_buffer(buffer &buffer, void *data, vk::DeviceSize size = VK_WHOLE_SIZE,
                               vk::DeviceSize offset = 0)
    {
        assert(buffer.mapped && data);
        if (size == VK_WHOLE_SIZE)
            memmove(buffer.mapped, data, buffer.buffer_size);
        else
        {
            char *mem_offset = static_cast<char *>(buffer.mapped);
            mem_offset += offset;
            memmove(mem_offset, data, size);
        }
    }

    /**
     * Copies "instance_size" bytes of data to the mapped buffer at an offset of index * alignment_size
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    inline void write_to_buffer_index(buffer &buffer, size_t instance_size, void *data, int index)
    {
        write_to_buffer(buffer, data, instance_size, index * buffer.alignment_size);
    }

    inline vk::Result flush_buffer(buffer &buffer, device &device, vk::DeviceSize size = VK_WHOLE_SIZE,
                                   vk::DeviceSize offset = 0)
    {
        return (vk::Result)vmaFlushAllocation(device.allocator, buffer.allocation, offset, size);
    }
    /**
     *  Flush the memory range at index * alignment_size of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    inline vk::Result flush_buffer_index(buffer &buffer, int index, device &device)
    {
        return flush_buffer(buffer, device, buffer.alignment_size, index * buffer.alignment_size);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return vk::Result of the invalidate call
     */
    inline vk::Result invalidate_buffer(buffer &buffer, device &device, vk::DeviceSize size = VK_WHOLE_SIZE,
                                        vk::DeviceSize offset = 0)
    {
        return (vk::Result)vmaInvalidateAllocation(device.allocator, buffer.allocation, offset, size);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignment_size
     *
     * @return vk::Result of the invalidate call
     */
    inline vk::Result invalidate_buffer_index(buffer &buffer, int index, device &device)
    {
        return invalidate_buffer(buffer, device, buffer.alignment_size, index * buffer.alignment_size);
    }

    struct buffer_mem_cache : public acul::mem_cache
    {
        explicit buffer_mem_cache(buffer &buffer, device &device)
            : acul::mem_cache{[this, &device]() { destroy_buffer(this->_buffer, device); }},
              _buffer{.mapped = buffer.mapped, .vk_buffer = buffer.vk_buffer, .allocation = buffer.allocation}
        {
            buffer = {};
        }

    private:
        buffer _buffer;
    };

    /// @brief Copy buffer from source to destination
    /// @param exec Single time execution
    /// @param device Device
    /// @param src_buffer Source buffer
    /// @param dst_buffer Destination buffer
    /// @param size Size of the buffer
    inline void copy_buffer(single_time_exec &exec, device &device, vk::Buffer src_buffer, vk::Buffer dst_buffer,
                            vk::DeviceSize size)
    {
        vk::BufferCopy copy_region(0, 0, size);
        exec.command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region, exec.loader);
    }

    /// @brief Copy buffer from source to destination
    /// @param device Device
    /// @param src_buffer Source buffer
    /// @param dst_buffer Destination buffer
    /// @param size Size of the buffer
    inline void copy_buffer(device &device, vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size)
    {
        single_time_exec exec{device};
        copy_buffer(exec, device, src_buffer, dst_buffer, size);
        exec.end();
    }

    struct gpu_upload_info
    {
        VmaAllocation allocation;
        vk::DeviceSize size;
        void *data = nullptr;
        buffer* staging = nullptr;

        acul::unique_function<void(single_time_exec &exec, bool)> on_upload;
        acul::unique_function<void(single_time_exec &, struct buffer &)> on_copy_staging;
        acul::unique_function<bool(void *, vk::DeviceSize)> on_staging_request;

        bool valid() const { return data && size > 0; }
    };

    inline acul::unique_function<void(single_time_exec &, struct buffer &)>
    make_copy_buffer_callback(device &device, buffer &dst_buffer, vk::DeviceSize size)
    {
        return [&device, &dst_buffer, size](single_time_exec &exec, buffer &staging) {
            copy_buffer(exec, device, staging.vk_buffer, dst_buffer.vk_buffer, size);
        };
    }

    inline acul::unique_function<void(single_time_exec &, struct buffer &)>
    make_move_buffer_callback(device &device, buffer &dst_buffer, vk::DeviceSize size)
    {
        return [&device, &dst_buffer, size](single_time_exec &exec, buffer &staging) {
            copy_buffer(exec, device, staging.vk_buffer, dst_buffer.vk_buffer, size);
        };
    }

    /**
     * Copies data to GPU buffer that is already mapped on the host.
     * It checks if the allocation is host visible and coherent, and if so, it just copies the data.
     * If not, it maps the allocation, copies the data, and then unmaps the allocation.
     * @param[in] upload_info Information about the upload.
     * @param[in] allocator The allocator to use for the allocation.
     * @param[in] mem_flags The memory property flags of the allocation.
     * @return True if the upload was successful, false otherwise.
     */
    APPLIB_API bool copy_data_to_gpu_buffer_host_visible(const gpu_upload_info &upload_info, VmaAllocator &allocator,
                                              VkMemoryPropertyFlags mem_flags);

    /**
     * Copies data to GPU buffer using a staging buffer.
     * Itt creates a staging buffer,
     * maps it, copies the data to the staging buffer, unmaps the staging buffer,
     * and then uses the staging buffer as a source of transfer to the buffer described previously.
     * @param[in] upload_info Information about the upload.
     * @param[in] device The device to use for the upload.
     * @return True if the upload was successful, false otherwise.
     */
    APPLIB_API bool copy_data_to_gpu_buffer_staging(const gpu_upload_info &upload_info, device &device);

    /**
     * Copies data to GPU buffer.
     * If the allocation is host visible, it copies the data directly to the buffer.
     * Otherwise, it uses a staging buffer to copy the data.
     * @param[in] upload_info Information about the upload.
     * @param[in] device The device to use for the upload.
     * @return True if the upload was successful, false otherwise.
     */
    APPLIB_API bool copy_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device);

    /**
     * Copies data to GPU buffer that is already mapped on the host.
     * It checks if the allocation is host visible and coherent, and if so, it just copies the data.
     * If not, it maps the allocation, copies the data, and then unmaps the allocation.
     * @param[in] upload_info Information about the upload.
     * @param[in] allocator The allocator to use for the allocation.
     * @param[in] mem_flags The memory property flags of the allocation.
     * @return True if the upload was successful, false otherwise.
     */
    bool move_data_to_gpu_buffer_host_visible(const gpu_upload_info &upload_info, VmaAllocator &allocator,
                                              VkMemoryPropertyFlags mem_flags);

    /**
     * Copies data to GPU buffer using a staging buffer.
     * Itt creates a staging buffer,
     * maps it, copies the data to the staging buffer, unmaps the staging buffer,
     * and then uses the staging buffer as a source of transfer to the buffer described previously.
     * @param[in] upload_info Information about the upload.
     * @param[in] device The device to use for the upload.
     * @return True if the upload was successful, false otherwise.
     */
    bool move_data_to_gpu_buffer_staging(const gpu_upload_info &upload_info, device &device);

    /**
     * Moves data to GPU buffer.
     * If the allocation is host visible, it moves the data directly to the buffer.
     * Otherwise, it uses a staging buffer to move the data.
     * @param[in] upload_info Information about the upload.
     * @param[in] device The device to use for the upload.
     * @return True if the upload was successful, false otherwise.
     */
    APPLIB_API bool move_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device);
} // namespace agrb
