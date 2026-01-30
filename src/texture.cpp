#include <agrb/buffer.hpp>
#include <agrb/texture.hpp>

namespace agrb
{
    bool create_texture_image_info(texture &texture, agrb::device &device)
    {
        vk::ImageCreateInfo image_info{};
        image_info.setImageType(vk::ImageType::e2D)
            .setFormat(texture.format)
            .setExtent(texture.image_extent)
            .setMipLevels(texture.mip_levels)
            .setArrayLayers(1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                      vk::ImageUsageFlagBits::eSampled)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setSharingMode(vk::SharingMode::eExclusive);

        auto create_info =
            make_alloc_info(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {}, vk::MemoryPropertyFlagBits::eDeviceLocal, 0.5f);
        return create_image(image_info, texture.image, texture.allocation, device.allocator, create_info);
    }

    void generate_texture_mipmaps(agrb::single_time_exec &exec, texture &texture)
    {
        vk::ImageMemoryBarrier barrier{};
        barrier.setImage(texture.image)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

        vk::Extent3D mip_extent = texture.image_extent;
        const u32 layer_count = texture.array_layers;

        for (u32 i = 1; i < texture.mip_levels; ++i)
        {
            barrier.subresourceRange.setBaseMipLevel(i - 1).setLevelCount(1).setLayerCount(layer_count);
            barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eTransferRead);

            exec.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1,
                                                &barrier, exec.loader);

            for (u32 layer = 0; layer < layer_count; ++layer)
            {
                vk::ImageBlit blit{};
                blit.setSrcOffsets(
                        {vk::Offset3D{0, 0, 0}, vk::Offset3D{int(mip_extent.width), int(mip_extent.height), 1}})
                    .setDstOffsets({vk::Offset3D{0, 0, 0}, vk::Offset3D{int(std::max(mip_extent.width / 2u, 1u)),
                                                                        int(std::max(mip_extent.height / 2u, 1u)), 1}})
                    .setSrcSubresource({vk::ImageAspectFlagBits::eColor, i - 1, layer, 1})
                    .setDstSubresource({vk::ImageAspectFlagBits::eColor, i, layer, 1});

                exec.command_buffer.blitImage(texture.image, vk::ImageLayout::eTransferSrcOptimal, texture.image,
                                              vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear,
                                              exec.loader);
            }

            // Transition previous mip to SHADER_READ
            barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            exec.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr,
                                                1, &barrier, exec.loader);

            mip_extent.width = std::max(mip_extent.width / 2u, 1u);
            mip_extent.height = std::max(mip_extent.height / 2u, 1u);
        }

        // Last mip level -> SHADER_READ
        barrier.subresourceRange.setBaseMipLevel(texture.mip_levels - 1).setLevelCount(1).setLayerCount(layer_count);
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        exec.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                            vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1,
                                            &barrier, exec.loader);
    }

    inline u32 calc_mipmap_levels(const vk::Extent3D &extent)
    {
        const u32 max_dim = std::max({extent.width, extent.height, extent.depth});
        return static_cast<u32>(std::floor(std::log2(max_dim))) + 1;
    }

    bool create_sampler(texture &texture, device &device)
    {
        vk::PhysicalDeviceProperties &properties = device.rd->get_device_properties();
        vk::SamplerCreateInfo sampler_create_info;
        sampler_create_info.setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setAnisotropyEnable(true)
            .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
            .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
            .setUnnormalizedCoordinates(false)
            .setCompareEnable(false)
            .setCompareOp(vk::CompareOp::eAlways)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setMipLodBias(0.0f)
            .setMinLod(0.0f)
            .setMaxLod(static_cast<f32>(texture.mip_levels));

        return device.vk_device.createSampler(&sampler_create_info, nullptr, &texture.sampler, device.loader) ==
               vk::Result::eSuccess;
    }

    bool allocate_texture(texture &texture, vk::ImageViewType image_type, void *pixels, device &device)
    {
        if (!pixels) return false;
        texture.mip_levels = texture.mip_levels == 0 ? calc_mipmap_levels(texture.image_extent) : texture.mip_levels;

        if (!create_texture_image_info(texture, device)) return false;

        gpu_upload_info upload_info;
        upload_info.allocation = texture.allocation;
        upload_info.data = pixels;
        upload_info.size = texture.size;
        upload_info.on_copy_staging = [&](single_time_exec &exec, buffer &staging) {
            if (!staging.vk_buffer) return;
            transition_image_layout(exec, texture.image, vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal, texture.mip_levels);
            copy_buffer_to_image(exec, staging.vk_buffer, texture.image, 1, texture.image_extent);
        };
        upload_info.on_upload = [&](single_time_exec &exec, bool) {
            if (texture.mip_levels > 1)
                generate_texture_mipmaps(exec, texture);
            else
                transition_image_layout(exec, texture.image, vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal, texture.mip_levels);
        };

        if (!move_data_to_gpu_buffer_staging(upload_info, device)) return false;

        vk::ImageViewCreateInfo image_view_create_info{};
        image_view_create_info.setImage(texture.image)
            .setViewType(image_type)
            .setFormat(texture.format)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, texture.mip_levels, 0, 1});
        texture.image_view = device.vk_device.createImageView(image_view_create_info, nullptr, device.loader);

        if (!create_sampler(texture, device)) return false;
        return true;
    }
} // namespace agrb