#pragma once
#include "device.hpp"

namespace agrb
{
    namespace defaults
    {
        constexpr vk::ClearValue clear_value = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
        constexpr vk::ClearValue clear_value_empty = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f));
        constexpr vk::ClearValue clear_value_stencil = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));
    } // namespace defaults

    struct fb_image
    {
        vk::Image image;
        VmaAllocation memory;
        u32 view_count = 1;
        union
        {
            vk::ImageView *views;
            vk::ImageView view;
        };
    };

    inline void destroy_fb_image(fb_image &image, device &dev)
    {
        if (image.view_count == 1)
            dev.vk_device.destroyImageView(image.view, nullptr, dev.loader);
        else
        {
            for (int i = 0; i < image.view_count; i++)
                dev.vk_device.destroyImageView(image.views[i], nullptr, dev.loader);
            acul::release(image.views);
        }
        if (image.image) vmaDestroyImage(dev.allocator, image.image, image.memory);
    }

    struct fb_image_slot
    {
        acul::vector<fb_image> attachments;
        vk::Framebuffer framebuffer;
    };

    inline void destroy_fb_image_slot(fb_image_slot &slot, device &dev)
    {
        for (auto &image : slot.attachments) destroy_fb_image(image, dev);
        if (slot.framebuffer) dev.vk_device.destroyFramebuffer(slot.framebuffer, nullptr, dev.loader);
    }

    struct fb_attachments
    {
        vk::Extent2D extent;
        fb_image_slot *images;
        u32 image_count;
        u32 attachment_count;

        f32 aspect_ratio() const { return static_cast<f32>(extent.width) / static_cast<f32>(extent.height); }
    };

    inline void destroy_fb_attachments(fb_attachments *attachments, device &dev)
    {
        for (u32 i = 0; i < attachments->image_count; i++) destroy_fb_image_slot(attachments->images[i], dev);
        acul::release(attachments);
    }

    struct framebuffer
    {
        vk::RenderPass render_pass = nullptr;
        vk::ClearValue *clear_values = nullptr;
        fb_attachments *attachments = nullptr;
    };

    inline void destroy_framebuffer(framebuffer &fb, device &dev)
    {
        dev.vk_device.destroyRenderPass(fb.render_pass, nullptr, dev.loader);
        acul::release(fb.clear_values);
        destroy_fb_attachments(fb.attachments, dev);
    }

    struct swapchain_create_info
    {
        fb_attachments *pAttachments = nullptr;
        device *pDevice = nullptr;
        vk::SwapchainKHR old_swapchain = nullptr;
        vk::SwapchainKHR *pSwapchain = nullptr;
        vk::Format image_format;

        swapchain_create_info &set_attachments(fb_attachments *pAttachments)
        {
            this->pAttachments = pAttachments;
            return *this;
        }

        swapchain_create_info &set_device(device *pDevice)
        {
            this->pDevice = pDevice;
            return *this;
        }

        swapchain_create_info &set_old_swapchain(vk::SwapchainKHR old_swapchain)
        {
            this->old_swapchain = old_swapchain;
            return *this;
        }

        swapchain_create_info &set_swapchain(vk::SwapchainKHR *pSwapchain)
        {
            this->pSwapchain = pSwapchain;
            return *this;
        }
    };

    APPLIB_API void create_swapchain(swapchain_create_info &create_info);

    inline void destroy_swapchain(fb_attachments *attachments, vk::SwapchainKHR swapchain, device &dev)
    {
        for (u32 i = 0; i < attachments->image_count; i++) attachments->images[i].attachments.front().image = nullptr;
        dev.vk_device.destroySwapchainKHR(swapchain, nullptr, dev.loader);
    }

    APPLIB_API bool create_fb_handles(framebuffer &fb, device &dev);

    inline bool create_fb_multi_image_view(fb_image &fb_image, u32 view_count, vk::ImageViewCreateInfo &view_info,
                                           device &dev)
    {
        fb_image.view_count = view_count;
        for (int i = 0; i < view_count; i++)
        {
            view_info.subresourceRange.baseArrayLayer = i;
            if (dev.vk_device.createImageView(&view_info, nullptr, &fb_image.views[i], dev.loader) !=
                vk::Result::eSuccess)
                return false;
        }
        return true;
    }
} // namespace agrb