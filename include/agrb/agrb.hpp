#pragma once

#include <acul/api.hpp>
#include <acul/memory/alloc.hpp>
#include <agrb/version.h>
#include <vulkan/vulkan.hpp>

#define AGRB_VENDOR_ID              0x407EC8
#define AGRB_OP_DOMAIN              0xD14A
#define AGRB_OP_ID_NOT_FOUND        0x0001
#define AGRB_OP_GPU_RESOURCE_FAILED 0x0002

#if VK_HEADER_VERSION > 290
namespace vk
{
    using namespace detail;
}
#endif

namespace agrb
{
    namespace detail
    {
        extern APPLIB_API struct device_library
        {
            vk::DynamicLoader vklib;
            vk::DispatchLoaderDynamic dispatch_loader;
        } *g_devlib;
    } // namespace detail

    inline void init_library() { detail::g_devlib = acul::alloc<detail::device_library>(); }

    inline void destroy_library() { acul::release(detail::g_devlib); }
} // namespace agrb
