#pragma once

#include "device.hpp"

namespace agrb
{
    struct adopted_device_create_info
    {
        vk::Instance instance;
        vk::PhysicalDevice physical_device;
        vk::Device vk_device;

        vk::Queue graphics_queue;
        u32 graphics_family_id = 0;

        vk::Queue compute_queue;
        u32 compute_family_id = 0;

        vk::Queue present_queue{};
        u32 present_family_id = 0;
        bool has_present_queue = false;

        // Runtime
        device_runtime_data *runtime_data = nullptr; // Optional external runtime storage.

        // Command pools
        bool create_command_pools = false;
        size_t graphics_primary_buffers = 5;
        size_t graphics_secondary_buffers = 10;
        size_t compute_primary_buffers = 2;
        size_t compute_secondary_buffers = 2;

        // Fence pool
        bool create_fence_pool = false;
        size_t fence_pool_size = 0;

        // Optional existing allocator.
        VmaAllocator allocator = nullptr;

        adopted_device_create_info &set_instance(vk::Instance value)
        {
            instance = value;
            return *this;
        }

        adopted_device_create_info &set_physical_device(vk::PhysicalDevice value)
        {
            physical_device = value;
            return *this;
        }

        adopted_device_create_info &set_vk_device(vk::Device value)
        {
            vk_device = value;
            return *this;
        }

        adopted_device_create_info &set_graphics_queue(vk::Queue queue, u32 family_id)
        {
            graphics_queue = queue;
            graphics_family_id = family_id;
            return *this;
        }

        adopted_device_create_info &set_compute_queue(vk::Queue queue, u32 family_id)
        {
            compute_queue = queue;
            compute_family_id = family_id;
            return *this;
        }

        adopted_device_create_info &set_present_queue(vk::Queue queue, u32 family_id)
        {
            present_queue = queue;
            present_family_id = family_id;
            has_present_queue = static_cast<bool>(queue);
            return *this;
        }

        adopted_device_create_info &set_runtime_data(device_runtime_data *value)
        {
            runtime_data = value;
            return *this;
        }

        adopted_device_create_info &set_create_command_pools(bool value)
        {
            create_command_pools = value;
            return *this;
        }

        adopted_device_create_info &set_graphics_command_buffers(size_t primary, size_t secondary)
        {
            graphics_primary_buffers = primary;
            graphics_secondary_buffers = secondary;
            return *this;
        }

        adopted_device_create_info &set_compute_command_buffers(size_t primary, size_t secondary)
        {
            compute_primary_buffers = primary;
            compute_secondary_buffers = secondary;
            return *this;
        }

        adopted_device_create_info &set_create_fence_pool(bool value)
        {
            create_fence_pool = value;
            return *this;
        }

        adopted_device_create_info &set_fence_pool_size(size_t value)
        {
            fence_pool_size = value;
            return *this;
        }

        adopted_device_create_info &set_allocator(VmaAllocator value)
        {
            allocator = value;
            return *this;
        }
    };

    APPLIB_API void initialize_adopted_device(device &device, const adopted_device_create_info &create_info);
    APPLIB_API void destroy_adopted_device(device &device);
    APPLIB_API bool create_adopted_allocator(device &device);
    APPLIB_API void destroy_adopted_allocator(device &device);
} // namespace agrb
