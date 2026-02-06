#pragma once

#include <acul/api.hpp>
#include <acul/memory/alloc.hpp>
#include <vulkan/vulkan.hpp>

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