#include <agrb/symbol_export.h>
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_CALL_PRE                 AGRB_EXPORT
#include <vk_mem_alloc.h>