#pragma once
#include <agrb/utils.hpp>

namespace agrb
{
    struct texture
    {
        vk::Image image;
        vk::ImageView image_view;
        vk::Sampler sampler;
        VmaAllocation allocation;
        u32 mip_levels;
        vk::Format format;
        vk::DeviceSize size;
        vk::Extent3D image_extent;
    };

    inline void swap(texture &a, texture &b)
    {
        std::swap(a.image, b.image);
        std::swap(a.image_view, b.image_view);
        std::swap(a.sampler, b.sampler);
        std::swap(a.allocation, b.allocation);
        std::swap(a.mip_levels, b.mip_levels);
        std::swap(a.format, b.format);
        std::swap(a.size, b.size);
        std::swap(a.image_extent, b.image_extent);
    }

    APPLIB_API VmaMemoryUsage get_texture_memory_usage(vk::ImageCreateInfo image_info, device &device,
                                                       vk::PhysicalDeviceMemoryProperties memory_properties);

    APPLIB_API bool create_texture_image_info(texture &texture, const vk::Buffer &buffer, device &device);

    APPLIB_API void generate_texture_mipmaps(single_time_exec &exec, texture &texture);

    APPLIB_API bool allocate_texture(texture &texture, vk::ImageViewType image_type, void *pixels, device &device);

    inline void destroy_texture(texture &texture, device &device)
    {
        if (texture.sampler) device.vk_device.destroySampler(texture.sampler, nullptr, device.loader);
        if (texture.image_view) device.vk_device.destroyImageView(texture.image_view, nullptr, device.loader);
        vmaDestroyImage(device.allocator, texture.image, texture.allocation);
    }
} // namespace agrb