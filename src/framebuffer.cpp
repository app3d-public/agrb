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
        return vk::PresentModeKHR::eImmediate;
    }

    static inline vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                                       vk::Extent2D extent)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            return capabilities.currentExtent;
        else
        {
            vk::Extent2D actual_extent = extent;
            actual_extent.width = std::max(capabilities.minImageExtent.width,
                                           std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height,
                                            std::min(capabilities.maxImageExtent.height, actual_extent.height));
            return actual_extent;
        }
    }

    void create_swapchain(swapchain_create_info &create_info)
    {
        auto &dev = *create_info.pDevice;
        auto swapchain_support = dev.query_swapchain_support();
        vk::SurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
        vk::PresentModeKHR present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
        vk::Extent2D extent = choose_swapchain_extent(swapchain_support.capabilities, create_info.pAttachments->extent);

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