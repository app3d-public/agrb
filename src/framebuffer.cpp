#include <agrb/framebuffer.hpp>

namespace agrb
{
    static inline vk::SurfaceFormatKHR
    choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR> &available_formats)
    {
        auto it = std::find_if(available_formats.begin(), available_formats.end(),
                               [](const vk::SurfaceFormatKHR &available_format) {
                                   return available_format.format == vk::Format::eB8G8R8A8Srgb &&
                                          available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                               });
        if (it != available_formats.end()) return *it;
        return available_formats[0];
    }

    static inline vk::PresentModeKHR
    choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR> &available_modes)
    {
        for (const auto &mode : available_modes)
            if (mode == vk::PresentModeKHR::eMailbox) return mode;
        return vk::PresentModeKHR::eFifo;
    }

    static inline vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                                       vk::Extent2D extent)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
        {
            if (capabilities.currentExtent.width > 0 && capabilities.currentExtent.height > 0)
                return capabilities.currentExtent;
        }

        vk::Extent2D actual_extent = extent;
        actual_extent.width = std::max(1u, actual_extent.width);
        actual_extent.height = std::max(1u, actual_extent.height);
        actual_extent.width = std::max(capabilities.minImageExtent.width,
                                       std::min(capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = std::max(capabilities.minImageExtent.height,
                                        std::min(capabilities.maxImageExtent.height, actual_extent.height));
        if (actual_extent.width == 0 || actual_extent.height == 0)
        {
            actual_extent.width = std::max(1u, actual_extent.width);
            actual_extent.height = std::max(1u, actual_extent.height);
        }
        return actual_extent;
    }

    static inline vk::Extent2D choose_scaled_swapchain_extent(const vk::SurfacePresentScalingCapabilitiesKHR &capabilities,
                                                              vk::Extent2D extent)
    {
        vk::Extent2D actual_extent = extent;
        actual_extent.width = std::max(1u, actual_extent.width);
        actual_extent.height = std::max(1u, actual_extent.height);
        actual_extent.width = std::max(capabilities.minScaledImageExtent.width,
                                       std::min(capabilities.maxScaledImageExtent.width, actual_extent.width));
        actual_extent.height = std::max(capabilities.minScaledImageExtent.height,
                                        std::min(capabilities.maxScaledImageExtent.height, actual_extent.height));
        return actual_extent;
    }

    static inline bool has_swapchain_maintenance1(device &dev)
    {
        if (!dev.rd) return false;
        return dev.rd->is_opt_extension_supported(vk::KHRSwapchainMaintenance1ExtensionName) ||
               dev.rd->is_opt_extension_supported(vk::EXTSwapchainMaintenance1ExtensionName);
    }

    static inline bool has_surface_maintenance1(device &dev)
    {
        if (!dev.rd) return false;
        return (dev.rd->is_opt_instance_extension_supported(vk::KHRGetSurfaceCapabilities2ExtensionName) &&
                dev.rd->is_opt_instance_extension_supported(vk::KHRSurfaceMaintenance1ExtensionName)) ||
               (dev.rd->is_opt_instance_extension_supported(vk::KHRGetSurfaceCapabilities2ExtensionName) &&
                dev.rd->is_opt_instance_extension_supported(vk::EXTSurfaceMaintenance1ExtensionName));
    }

    static bool query_present_scaling_capabilities(device &dev, vk::PresentModeKHR present_mode,
                                                   vk::SurfacePresentScalingCapabilitiesKHR &capabilities)
    {
        if (!has_surface_maintenance1(dev)) return false;

        vk::SurfacePresentModeKHR present_mode_info;
        present_mode_info.setPresentMode(present_mode);

        vk::PhysicalDeviceSurfaceInfo2KHR surface_info;
        surface_info.setSurface(dev.surface).setPNext(&present_mode_info);

        vk::SurfaceCapabilities2KHR capabilities2;
        capabilities2.pNext = &capabilities;

        const auto result = dev.physical_device.getSurfaceCapabilities2KHR(&surface_info, &capabilities2, dev.loader);
        return result == vk::Result::eSuccess;
    }

    static bool try_enable_present_scaling(device &dev, swapchain_create_info &create_info,
                                           vk::PresentModeKHR present_mode,
                                           vk::SwapchainPresentScalingCreateInfoKHR &scaling_info,
                                           vk::Extent2D &extent)
    {
        if (!create_info.present_scaling) return false;
        if (!has_swapchain_maintenance1(dev)) return false;

        vk::SurfacePresentScalingCapabilitiesKHR scaling_capabilities;
        if (!query_present_scaling_capabilities(dev, present_mode, scaling_capabilities)) return false;

        const bool scaling_supported =
            static_cast<bool>(scaling_capabilities.supportedPresentScaling & create_info.present_scaling_behavior);
        const bool gravity_x_supported =
            static_cast<bool>(scaling_capabilities.supportedPresentGravityX & create_info.present_gravity_x);
        const bool gravity_y_supported =
            static_cast<bool>(scaling_capabilities.supportedPresentGravityY & create_info.present_gravity_y);
        if (!scaling_supported || !gravity_x_supported || !gravity_y_supported) return false;

        extent = choose_scaled_swapchain_extent(scaling_capabilities, create_info.pAttachments->extent);
        scaling_info.setScalingBehavior(create_info.present_scaling_behavior)
            .setPresentGravityX(create_info.present_gravity_x)
            .setPresentGravityY(create_info.present_gravity_y);
        return true;
    }

    void create_swapchain(swapchain_create_info &create_info)
    {
        auto &dev = *create_info.pDevice;
        auto swapchain_support = dev.query_swapchain_support();
        vk::SurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
        vk::PresentModeKHR present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
        vk::Extent2D extent = choose_swapchain_extent(swapchain_support.capabilities, create_info.pAttachments->extent);
        vk::SwapchainPresentScalingCreateInfoKHR scaling_info;
        const bool present_scaling_applied =
            try_enable_present_scaling(dev, create_info, present_mode, scaling_info, extent);
        if (present_scaling_applied) scaling_info.setPNext(nullptr);

        u32 image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support.capabilities.maxImageCount)
            image_count = swapchain_support.capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR khr_create_info;
        khr_create_info.setSurface(create_info.pDevice->surface)
            .setMinImageCount(image_count)
            .setImageFormat(surface_format.format)
            .setImageColorSpace(surface_format.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

        auto &queues = dev.rd->queues;
        u32 indices[2] = {queues.graphics.family_id.value(), queues.present.family_id.value()};
        if (indices[0] != indices[1])
        {
            khr_create_info.setImageSharingMode(vk::SharingMode::eConcurrent)
                .setQueueFamilyIndexCount(2)
                .setPQueueFamilyIndices(indices);
        }
        else
            khr_create_info.setImageSharingMode(vk::SharingMode::eExclusive);

        khr_create_info.setPreTransform(swapchain_support.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(present_mode)
            .setClipped(true)
            .setOldSwapchain(create_info.old_swapchain);
        if (present_scaling_applied) khr_create_info.setPNext(&scaling_info);

        *create_info.pSwapchain = dev.vk_device.createSwapchainKHR(khr_create_info, nullptr, dev.loader);
        auto new_images = dev.vk_device.getSwapchainImagesKHR(*create_info.pSwapchain, dev.loader);
        create_info.pAttachments->image_count = new_images.size();
        create_info.pAttachments->images = acul::alloc_n<fb_image_slot>(new_images.size());
        auto *images = create_info.pAttachments->images;
        for (int i = 0; i < new_images.size(); ++i)
        {
            images[i].attachments.resize(create_info.pAttachments->attachment_count);
            images[i].attachments.front().image = new_images[i];
        }
        create_info.image_format = surface_format.format;
        create_info.pAttachments->extent = extent;
    }

    bool create_fb_handles(framebuffer *fb, device &dev)
    {
        assert(fb->attachments);
        auto &fb_attachments = *fb->attachments;
        for (int i = 0; i < fb_attachments.image_count; ++i)
        {
            fb_image_slot &image = fb_attachments.images[i];
            vk::ImageView attachments[fb_attachments.attachment_count];
            int out = 0;
            for (int slot = 0; slot < static_cast<int>(image.attachments.size()); ++slot)
                for (auto &attachment : image.attachments[slot].view_group) attachments[out++] = attachment;

            int fb_id = 0;
            for (auto &fb_dst : image.fb_group)
            {
                vk::FramebufferCreateInfo framebuffer_info;
                framebuffer_info.setRenderPass(fb->rp_group.at(fb_id))
                    .setAttachmentCount(fb_attachments.attachment_count)
                    .setPAttachments(attachments)
                    .setWidth(fb_attachments.extent.width)
                    .setHeight(fb_attachments.extent.height)
                    .setLayers(1);
                if (dev.vk_device.createFramebuffer(&framebuffer_info, nullptr, &fb_dst, dev.loader) !=
                    vk::Result::eSuccess)
                    return false;
                ++fb_id;
            }
        }
        return true;
    }
} // namespace agrb
