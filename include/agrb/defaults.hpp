#pragma once

#include <vulkan/vulkan.hpp>

namespace agrb
{
    namespace defaults
    {
        // clear values
        constexpr vk::ClearValue clear_value = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
        constexpr vk::ClearValue clear_value_empty = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f));
        constexpr vk::ClearValue clear_value_stencil = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

        // Subresource ranges
        constexpr vk::ImageSubresourceRange subresource_range_color =
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        constexpr vk::ImageSubresourceRange subresource_range_depth =
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1);
    } // namespace defaults
} // namespace agrb