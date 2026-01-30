#pragma once
#include <acul/memory/paos.hpp>
#include "device.hpp"

namespace agrb
{
    struct fb_image
    {
        vk::Image image;
        VmaAllocation memory;
        acul::paos<vk::ImageView> view_group;

        vk::ImageView &get_view(u32 view_id) { return view_group[view_id]; }
        vk::ImageView &get_view() { return view_group.value(); }
    };

    inline void destroy_fb_image(fb_image &image, device &dev)
    {
        for (const auto &view : image.view_group) dev.vk_device.destroyImageView(view, nullptr, dev.loader);
        image.view_group.deallocate();
        if (image.image) vmaDestroyImage(dev.allocator, image.image, image.memory);
    }

    struct fb_image_slot
    {
        acul::vector<fb_image> attachments;
        acul::paos<vk::Framebuffer> fb_group;
    };

    inline void destroy_fb_image_slot(fb_image_slot &slot, device &dev)
    {
        for (auto &image : slot.attachments) destroy_fb_image(image, dev);
        for (const auto &fb : slot.fb_group) dev.vk_device.destroyFramebuffer(fb, nullptr, dev.loader);
        slot.fb_group.deallocate();
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
        if (!attachments) return;
        for (u32 i = 0; i < attachments->image_count; i++) destroy_fb_image_slot(attachments->images[i], dev);
        acul::release(attachments);
    }

    struct framebuffer
    {
        acul::paos<vk::RenderPass> rp_group;
        vk::ClearValue *clear_values = nullptr;
        fb_attachments *attachments = nullptr;

        vk::RenderPass get_rp(u32 rp_id) const { return rp_group[rp_id]; }
        vk::RenderPass get_rp() const { return rp_group.value(); }

        vk::Framebuffer get_fb(u32 frame_id, u32 fb_id) const { return attachments->images[frame_id].fb_group[fb_id]; }
        vk::Framebuffer get_fb(u32 frame_id) const { return attachments->images[frame_id].fb_group.value(); }
    };

    inline void destroy_framebuffer(framebuffer &fb, device &dev)
    {
        for (const auto &render_pass : fb.rp_group)
            if (render_pass) dev.vk_device.destroyRenderPass(render_pass, nullptr, dev.loader);
        fb.rp_group.deallocate();
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

    APPLIB_API bool create_fb_handles(framebuffer *fb, device &dev);

    inline bool create_fb_multi_image_view(fb_image &fb_image, vk::ImageViewCreateInfo &view_info, device &dev)
    {
        assert(fb_image.view_group.size() > 0);
        int i = 0;
        for (auto &view : fb_image.view_group)
        {
            view_info.subresourceRange.baseArrayLayer = i++;
            if (dev.vk_device.createImageView(&view_info, nullptr, &view, dev.loader) != vk::Result::eSuccess)
                return false;
        }
        return true;
    }
} // namespace agrb