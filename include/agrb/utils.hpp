#pragma once

/****************************************************
 *  Helper device functions
 *****************************************************/

#include <acul/pair.hpp>
#include "device.hpp"

namespace agrb
{
    inline size_t get_alignment(size_t size, size_t min_offset)
    {
        return min_offset > 0 ? (size + min_offset - 1) & ~(min_offset - 1) : size;
    }

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

    inline void copy_buffer_to_image(single_time_exec &exec, vk::Buffer buffer, vk::Image image, u32 layer_count,
                                     vk::Extent3D image_extent, vk::Offset3D image_offset = {0, 0, 0})
    {
        vk::BufferImageCopy region{};
        region.setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layer_count})
            .setImageOffset(image_offset)
            .setImageExtent(image_extent);
        exec.command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region,
                                              exec.loader);
    }

    inline vk::Result copy_buffer_to_image(device &device, vk::Buffer buffer, vk::Image image, u32 layer_count,
                                           vk::Extent3D image_extent, vk::Offset3D offset = {0, 0, 0})
    {
        single_time_exec exec{device};
        copy_buffer_to_image(exec, buffer, image, layer_count, image_extent, offset);
        return exec.end();
    }

    inline VmaAllocationCreateInfo make_alloc_info(VmaMemoryUsage usage, vk::MemoryPropertyFlags required_flags,
                                                   vk::MemoryPropertyFlags preferred_flags, f32 priority)
    {
        return {.usage = usage,
                .requiredFlags = static_cast<VkMemoryPropertyFlags>(required_flags),
                .preferredFlags = static_cast<VkMemoryPropertyFlags>(preferred_flags),
                .priority = priority};
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
        void *data;

        std::function<void(single_time_exec &exec, bool)> on_upload;
        std::function<void(single_time_exec &, struct buffer &)> on_copy_staging;

        bool valid() const { return data && size > 0; }
    };

    /**
     * Copies data to GPU buffer that is already mapped on the host.
     * It checks if the allocation is host visible and coherent, and if so, it just copies the data.
     * If not, it maps the allocation, copies the data, and then unmaps the allocation.
     * @param[in] upload_info Information about the upload.
     * @param[in] allocator The allocator to use for the allocation.
     * @param[in] mem_flags The memory property flags of the allocation.
     * @return True if the upload was successful, false otherwise.
     */
    bool copy_data_to_gpu_buffer_host_visible(const gpu_upload_info &upload_info, VmaAllocator &allocator,
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
    bool copy_data_to_gpu_buffer_staging(const gpu_upload_info &upload_info, device &device);

    /**
     * Copies data to GPU buffer.
     * If the allocation is host visible, it copies the data directly to the buffer.
     * Otherwise, it uses a staging buffer to copy the data.
     * @param[in] upload_info Information about the upload.
     * @param[in] device The device to use for the upload.
     * @return True if the upload was successful, false otherwise.
     */
    bool copy_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device);

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
    bool move_data_to_gpu_buffer(const gpu_upload_info &upload_info, device &device);

    /// @brief Transit image layout from one layout to another one using pipeline memory barrier
    /// @param device Device
    /// @param image Image
    /// @param old_layout Current layout
    /// @param new_layout New layout
    /// @param mip_levels Image mip level
    APPLIB_API void transition_image_layout(single_time_exec &exec, vk::Image image, vk::ImageLayout old_layout,
                                            vk::ImageLayout new_layout, u32 mip_levels);

    inline vk::Result transition_image_layout(device &device, vk::Image image, vk::ImageLayout old_layout,
                                              vk::ImageLayout new_layout, u32 mip_levels)
    {
        single_time_exec exec{device};
        transition_image_layout(exec, image, old_layout, new_layout, mip_levels);
        return exec.end();
    }

    /// @brief Copy image to vk buffer
    /// @param device Device
    /// @param buffer Source buffer
    /// @param image Destination image
    /// @param dimensions Image dimensions
    /// @param layer_count Layer count
    /// @param offset Offset
    inline void copy_image_to_buffer(device &device, vk::Buffer buffer, vk::Image image, acul::point2D<u32> dimensions,
                                     u32 layerCount, acul::point2D<int> offset = {0, 0})
    {
        single_time_exec info{device};
        vk::BufferImageCopy region{};
        region.setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layerCount})
            .setImageOffset({offset.x, offset.y, 0})
            .setImageExtent({dimensions.x, dimensions.y, 1});
        info.command_buffer.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, buffer, 1, &region,
                                              info.loader);
        info.end();
    }

    /**
     * @brief Allocates a Vulkan image and binds it to a VMA allocation.
     *
     * @param image_info Image creation info.
     * @param image Allocated image.
     * @param allocated Allocation created by VMA.
     * @param allocator VMA allocator.
     * @param alloc_info Allocation info.
     *
     * @return true if image allocation succeeded, false otherwise.
     */
    inline bool create_image(const vk::ImageCreateInfo &image_info, vk::Image &image, VmaAllocation &allocation,
                             VmaAllocator &allocator, VmaAllocationCreateInfo alloc_info)
    {
        return vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo *>(&image_info), &alloc_info,
                              reinterpret_cast<VkImage *>(&image), &allocation, nullptr) == VK_SUCCESS;
    }

    /**
     * @brief Clamp a 2D rectangle to fit within a given 2D extent.
     *
     * @param r The rectangle to clamp.
     * @param e The extent to clamp to.
     *
     * @return A new rectangle that is clamped to fit within the given extent.
     */
    inline vk::Rect2D clamp_rect_to_extent(vk::Rect2D r, vk::Extent2D e)
    {
        i32 x0 = std::max(0, r.offset.x);
        i32 y0 = std::max(0, r.offset.y);

        i32 x1 = std::min<i32>((i32)e.width, r.offset.x + (i32)r.extent.width);
        i32 y1 = std::min<i32>((i32)e.height, r.offset.y + (i32)r.extent.height);

        u32 w = (x1 > x0) ? (u32)(x1 - x0) : 0u;
        u32 h = (y1 > y0) ? (u32)(y1 - y0) : 0u;

        return vk::Rect2D{vk::Offset2D{x0, y0}, vk::Extent2D{w, h}};
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
} // namespace agrb