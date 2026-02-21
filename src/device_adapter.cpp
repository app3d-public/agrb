#include <acul/exception/exception.hpp>
#include <agrb/device_adapter.hpp>

namespace agrb
{
    namespace
    {
        static void init_dispatch_loader(device &device)
        {
            auto &loader = device.loader;
#ifdef VK_NO_PROTOTYPES
            loader.init(device.instance);
#else
            loader.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
            loader.vkDestroyInstance = &vkDestroyInstance;
            loader.vkEnumeratePhysicalDevices = &vkEnumeratePhysicalDevices;
            loader.vkGetPhysicalDeviceFeatures = &vkGetPhysicalDeviceFeatures;
            loader.vkGetPhysicalDeviceFormatProperties = &vkGetPhysicalDeviceFormatProperties;
            loader.vkGetPhysicalDeviceImageFormatProperties = &vkGetPhysicalDeviceImageFormatProperties;
            loader.vkGetPhysicalDeviceProperties = &vkGetPhysicalDeviceProperties;
            loader.vkGetPhysicalDeviceQueueFamilyProperties = &vkGetPhysicalDeviceQueueFamilyProperties;
            loader.vkGetPhysicalDeviceMemoryProperties = &vkGetPhysicalDeviceMemoryProperties;
            loader.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
            loader.vkCreateDevice = &vkCreateDevice;
            loader.vkDestroyDevice = &vkDestroyDevice;
            loader.vkEnumerateDeviceExtensionProperties = &vkEnumerateDeviceExtensionProperties;
            loader.vkEnumerateDeviceLayerProperties = &vkEnumerateDeviceLayerProperties;
            loader.vkGetDeviceQueue = &vkGetDeviceQueue;
            loader.vkQueueSubmit = &vkQueueSubmit;
            loader.vkQueueWaitIdle = &vkQueueWaitIdle;
            loader.vkDeviceWaitIdle = &vkDeviceWaitIdle;
            loader.vkAllocateMemory = &vkAllocateMemory;
            loader.vkFreeMemory = &vkFreeMemory;
            loader.vkMapMemory = &vkMapMemory;
            loader.vkUnmapMemory = &vkUnmapMemory;
            loader.vkFlushMappedMemoryRanges = &vkFlushMappedMemoryRanges;
            loader.vkInvalidateMappedMemoryRanges = &vkInvalidateMappedMemoryRanges;
            loader.vkGetDeviceMemoryCommitment = &vkGetDeviceMemoryCommitment;
            loader.vkBindBufferMemory = &vkBindBufferMemory;
            loader.vkBindImageMemory = &vkBindImageMemory;
            loader.vkGetBufferMemoryRequirements = &vkGetBufferMemoryRequirements;
            loader.vkGetImageMemoryRequirements = &vkGetImageMemoryRequirements;
            loader.vkGetImageSparseMemoryRequirements = &vkGetImageSparseMemoryRequirements;
            loader.vkGetPhysicalDeviceSparseImageFormatProperties = &vkGetPhysicalDeviceSparseImageFormatProperties;
            loader.vkQueueBindSparse = &vkQueueBindSparse;
            loader.vkCreateFence = &vkCreateFence;
            loader.vkDestroyFence = &vkDestroyFence;
            loader.vkResetFences = &vkResetFences;
            loader.vkGetFenceStatus = &vkGetFenceStatus;
            loader.vkWaitForFences = &vkWaitForFences;
            loader.vkCreateSemaphore = &vkCreateSemaphore;
            loader.vkDestroySemaphore = &vkDestroySemaphore;
            loader.vkCreateQueryPool = &vkCreateQueryPool;
            loader.vkDestroyQueryPool = &vkDestroyQueryPool;
            loader.vkGetQueryPoolResults = &vkGetQueryPoolResults;
            loader.vkCreateBuffer = &vkCreateBuffer;
            loader.vkDestroyBuffer = &vkDestroyBuffer;
            loader.vkCreateImage = &vkCreateImage;
            loader.vkDestroyImage = &vkDestroyImage;
            loader.vkGetImageSubresourceLayout = &vkGetImageSubresourceLayout;
            loader.vkCreateImageView = &vkCreateImageView;
            loader.vkDestroyImageView = &vkDestroyImageView;
            loader.vkCreateCommandPool = &vkCreateCommandPool;
            loader.vkDestroyCommandPool = &vkDestroyCommandPool;
            loader.vkResetCommandPool = &vkResetCommandPool;
            loader.vkAllocateCommandBuffers = &vkAllocateCommandBuffers;
            loader.vkFreeCommandBuffers = &vkFreeCommandBuffers;
            loader.vkBeginCommandBuffer = &vkBeginCommandBuffer;
            loader.vkEndCommandBuffer = &vkEndCommandBuffer;
            loader.vkResetCommandBuffer = &vkResetCommandBuffer;
            loader.vkCmdCopyBuffer = &vkCmdCopyBuffer;
            loader.vkCmdCopyImage = &vkCmdCopyImage;
            loader.vkCmdCopyBufferToImage = &vkCmdCopyBufferToImage;
            loader.vkCmdCopyImageToBuffer = &vkCmdCopyImageToBuffer;
            loader.vkCmdUpdateBuffer = &vkCmdUpdateBuffer;
            loader.vkCmdFillBuffer = &vkCmdFillBuffer;
            loader.vkCmdPipelineBarrier = &vkCmdPipelineBarrier;
            loader.vkCmdBeginQuery = &vkCmdBeginQuery;
            loader.vkCmdEndQuery = &vkCmdEndQuery;
            loader.vkCmdResetQueryPool = &vkCmdResetQueryPool;
            loader.vkCmdWriteTimestamp = &vkCmdWriteTimestamp;
            loader.vkCmdCopyQueryPoolResults = &vkCmdCopyQueryPoolResults;
            loader.vkCmdExecuteCommands = &vkCmdExecuteCommands;
            loader.vkCreateEvent = &vkCreateEvent;
            loader.vkDestroyEvent = &vkDestroyEvent;
            loader.vkGetEventStatus = &vkGetEventStatus;
            loader.vkSetEvent = &vkSetEvent;
            loader.vkResetEvent = &vkResetEvent;
            loader.vkCreateBufferView = &vkCreateBufferView;
            loader.vkDestroyBufferView = &vkDestroyBufferView;
            loader.vkCreateShaderModule = &vkCreateShaderModule;
            loader.vkDestroyShaderModule = &vkDestroyShaderModule;
            loader.vkCreatePipelineCache = &vkCreatePipelineCache;
            loader.vkDestroyPipelineCache = &vkDestroyPipelineCache;
            loader.vkGetPipelineCacheData = &vkGetPipelineCacheData;
            loader.vkMergePipelineCaches = &vkMergePipelineCaches;
            loader.vkCreateComputePipelines = &vkCreateComputePipelines;
            loader.vkDestroyPipeline = &vkDestroyPipeline;
            loader.vkCreatePipelineLayout = &vkCreatePipelineLayout;
            loader.vkDestroyPipelineLayout = &vkDestroyPipelineLayout;
            loader.vkCreateSampler = &vkCreateSampler;
            loader.vkDestroySampler = &vkDestroySampler;
            loader.vkCreateDescriptorSetLayout = &vkCreateDescriptorSetLayout;
            loader.vkDestroyDescriptorSetLayout = &vkDestroyDescriptorSetLayout;
            loader.vkCreateDescriptorPool = &vkCreateDescriptorPool;
            loader.vkDestroyDescriptorPool = &vkDestroyDescriptorPool;
            loader.vkResetDescriptorPool = &vkResetDescriptorPool;
            loader.vkAllocateDescriptorSets = &vkAllocateDescriptorSets;
            loader.vkFreeDescriptorSets = &vkFreeDescriptorSets;
            loader.vkUpdateDescriptorSets = &vkUpdateDescriptorSets;
            loader.vkCmdBindPipeline = &vkCmdBindPipeline;
            loader.vkCmdBindDescriptorSets = &vkCmdBindDescriptorSets;
            loader.vkCmdClearColorImage = &vkCmdClearColorImage;
            loader.vkCmdDispatch = &vkCmdDispatch;
            loader.vkCmdDispatchIndirect = &vkCmdDispatchIndirect;
            loader.vkCmdSetEvent = &vkCmdSetEvent;
            loader.vkCmdResetEvent = &vkCmdResetEvent;
            loader.vkCmdWaitEvents = &vkCmdWaitEvents;
            loader.vkCmdPushConstants = &vkCmdPushConstants;
            loader.vkCreateGraphicsPipelines = &vkCreateGraphicsPipelines;
            loader.vkCreateFramebuffer = &vkCreateFramebuffer;
            loader.vkDestroyFramebuffer = &vkDestroyFramebuffer;
            loader.vkCreateRenderPass = &vkCreateRenderPass;
            loader.vkDestroyRenderPass = &vkDestroyRenderPass;
            loader.vkGetRenderAreaGranularity = &vkGetRenderAreaGranularity;
            loader.vkCmdSetViewport = &vkCmdSetViewport;
            loader.vkCmdSetScissor = &vkCmdSetScissor;
            loader.vkCmdSetLineWidth = &vkCmdSetLineWidth;
            loader.vkCmdSetDepthBias = &vkCmdSetDepthBias;
            loader.vkCmdSetBlendConstants = &vkCmdSetBlendConstants;
            loader.vkCmdSetDepthBounds = &vkCmdSetDepthBounds;
            loader.vkCmdSetStencilCompareMask = &vkCmdSetStencilCompareMask;
            loader.vkCmdSetStencilWriteMask = &vkCmdSetStencilWriteMask;
            loader.vkCmdSetStencilReference = &vkCmdSetStencilReference;
            loader.vkCmdBindIndexBuffer = &vkCmdBindIndexBuffer;
            loader.vkCmdBindVertexBuffers = &vkCmdBindVertexBuffers;
            loader.vkCmdDraw = &vkCmdDraw;
            loader.vkCmdDrawIndexed = &vkCmdDrawIndexed;
            loader.vkCmdDrawIndirect = &vkCmdDrawIndirect;
            loader.vkCmdDrawIndexedIndirect = &vkCmdDrawIndexedIndirect;
            loader.vkCmdBlitImage = &vkCmdBlitImage;
            loader.vkCmdClearDepthStencilImage = &vkCmdClearDepthStencilImage;
            loader.vkCmdClearAttachments = &vkCmdClearAttachments;
            loader.vkCmdResolveImage = &vkCmdResolveImage;
            loader.vkCmdBeginRenderPass = &vkCmdBeginRenderPass;
            loader.vkCmdNextSubpass = &vkCmdNextSubpass;
            loader.vkCmdEndRenderPass = &vkCmdEndRenderPass;

            //=== VK_VERSION_1_1 ===
            loader.vkBindBufferMemory2 = &vkBindBufferMemory2;
            loader.vkBindImageMemory2 = &vkBindImageMemory2;
            loader.vkGetDeviceGroupPeerMemoryFeatures = &vkGetDeviceGroupPeerMemoryFeatures;
            loader.vkCmdSetDeviceMask = &vkCmdSetDeviceMask;
            loader.vkEnumeratePhysicalDeviceGroups = &vkEnumeratePhysicalDeviceGroups;
            loader.vkGetImageMemoryRequirements2 = &vkGetImageMemoryRequirements2;
            loader.vkGetBufferMemoryRequirements2 = &vkGetBufferMemoryRequirements2;
            loader.vkGetImageSparseMemoryRequirements2 = &vkGetImageSparseMemoryRequirements2;
            loader.vkGetPhysicalDeviceFeatures2 = &vkGetPhysicalDeviceFeatures2;
            loader.vkGetPhysicalDeviceProperties2 = &vkGetPhysicalDeviceProperties2;
            loader.vkGetPhysicalDeviceFormatProperties2 = &vkGetPhysicalDeviceFormatProperties2;
            loader.vkGetPhysicalDeviceImageFormatProperties2 = &vkGetPhysicalDeviceImageFormatProperties2;
            loader.vkGetPhysicalDeviceQueueFamilyProperties2 = &vkGetPhysicalDeviceQueueFamilyProperties2;
            loader.vkGetPhysicalDeviceMemoryProperties2 = &vkGetPhysicalDeviceMemoryProperties2;
            loader.vkGetPhysicalDeviceSparseImageFormatProperties2 = &vkGetPhysicalDeviceSparseImageFormatProperties2;
            loader.vkTrimCommandPool = &vkTrimCommandPool;
            loader.vkGetDeviceQueue2 = &vkGetDeviceQueue2;
            loader.vkGetPhysicalDeviceExternalBufferProperties = &vkGetPhysicalDeviceExternalBufferProperties;
            loader.vkGetPhysicalDeviceExternalFenceProperties = &vkGetPhysicalDeviceExternalFenceProperties;
            loader.vkGetPhysicalDeviceExternalSemaphoreProperties = &vkGetPhysicalDeviceExternalSemaphoreProperties;
            loader.vkCmdDispatchBase = &vkCmdDispatchBase;
            loader.vkCreateDescriptorUpdateTemplate = &vkCreateDescriptorUpdateTemplate;
            loader.vkDestroyDescriptorUpdateTemplate = &vkDestroyDescriptorUpdateTemplate;
            loader.vkUpdateDescriptorSetWithTemplate = &vkUpdateDescriptorSetWithTemplate;
            loader.vkGetDescriptorSetLayoutSupport = &vkGetDescriptorSetLayoutSupport;
            loader.vkCreateSamplerYcbcrConversion = &vkCreateSamplerYcbcrConversion;
            loader.vkDestroySamplerYcbcrConversion = &vkDestroySamplerYcbcrConversion;

            //=== VK_VERSION_1_2 ===
            loader.vkResetQueryPool = &vkResetQueryPool;
            loader.vkGetSemaphoreCounterValue = &vkGetSemaphoreCounterValue;
            loader.vkWaitSemaphores = &vkWaitSemaphores;
            loader.vkSignalSemaphore = &vkSignalSemaphore;
            loader.vkGetBufferDeviceAddress = &vkGetBufferDeviceAddress;
            loader.vkGetBufferOpaqueCaptureAddress = &vkGetBufferOpaqueCaptureAddress;
            loader.vkGetDeviceMemoryOpaqueCaptureAddress = &vkGetDeviceMemoryOpaqueCaptureAddress;
            loader.vkCmdDrawIndirectCount = &vkCmdDrawIndirectCount;
            loader.vkCmdDrawIndexedIndirectCount = &vkCmdDrawIndexedIndirectCount;
            loader.vkCreateRenderPass2 = &vkCreateRenderPass2;
            loader.vkCmdBeginRenderPass2 = &vkCmdBeginRenderPass2;
            loader.vkCmdNextSubpass2 = &vkCmdNextSubpass2;
            loader.vkCmdEndRenderPass2 = &vkCmdEndRenderPass2;

            //=== VK_EXT_debug_utils ===
#ifndef VK_ONLY_EXPORTED_PROTOTYPES
            loader.vkSetDebugUtilsObjectNameEXT = &vkSetDebugUtilsObjectNameEXT;
            loader.vkSetDebugUtilsObjectTagEXT = &vkSetDebugUtilsObjectTagEXT;
            loader.vkQueueBeginDebugUtilsLabelEXT = &vkQueueBeginDebugUtilsLabelEXT;
            loader.vkQueueEndDebugUtilsLabelEXT = &vkQueueEndDebugUtilsLabelEXT;
            loader.vkQueueInsertDebugUtilsLabelEXT = &vkQueueInsertDebugUtilsLabelEXT;
            loader.vkCmdBeginDebugUtilsLabelEXT = &vkCmdBeginDebugUtilsLabelEXT;
            loader.vkCmdEndDebugUtilsLabelEXT = &vkCmdEndDebugUtilsLabelEXT;
            loader.vkCmdInsertDebugUtilsLabelEXT = &vkCmdInsertDebugUtilsLabelEXT;
            loader.vkCreateDebugUtilsMessengerEXT = &vkCreateDebugUtilsMessengerEXT;
            loader.vkDestroyDebugUtilsMessengerEXT = &vkDestroyDebugUtilsMessengerEXT;
            loader.vkSubmitDebugUtilsMessageEXT = &vkSubmitDebugUtilsMessageEXT;

            //=== VK_EXT_descriptor_heap ===
            loader.vkWriteSamplerDescriptorsEXT = &vkWriteSamplerDescriptorsEXT;
            loader.vkWriteResourceDescriptorsEXT = &vkWriteResourceDescriptorsEXT;
            loader.vkCmdBindSamplerHeapEXT = &vkCmdBindSamplerHeapEXT;
            loader.vkCmdBindResourceHeapEXT = &vkCmdBindResourceHeapEXT;
            loader.vkCmdPushDataEXT = &vkCmdPushDataEXT;
            loader.vkGetImageOpaqueCaptureDataEXT = &vkGetImageOpaqueCaptureDataEXT;
            loader.vkGetPhysicalDeviceDescriptorSizeEXT = &vkGetPhysicalDeviceDescriptorSizeEXT;
            loader.vkRegisterCustomBorderColorEXT = &vkRegisterCustomBorderColorEXT;
            loader.vkUnregisterCustomBorderColorEXT = &vkUnregisterCustomBorderColorEXT;
            loader.vkGetTensorOpaqueCaptureDataARM = &vkGetTensorOpaqueCaptureDataARM;

            //=== VK_EXT_sample_locations ===
            loader.vkCmdSetSampleLocationsEXT = &vkCmdSetSampleLocationsEXT;
            loader.vkGetPhysicalDeviceMultisamplePropertiesEXT = &vkGetPhysicalDeviceMultisamplePropertiesEXT;

            //=== VK_EXT_buffer_device_address ===
            loader.vkGetBufferDeviceAddressEXT = &vkGetBufferDeviceAddressEXT;
            if (!vkGetBufferDeviceAddress) loader.vkGetBufferDeviceAddress = loader.vkGetBufferDeviceAddressEXT;

            //=== VK_EXT_tooling_info ===
            loader.vkGetPhysicalDeviceToolPropertiesEXT = &vkGetPhysicalDeviceToolPropertiesEXT;
            if (!vkGetPhysicalDeviceToolProperties)
                loader.vkGetPhysicalDeviceToolProperties = loader.vkGetPhysicalDeviceToolPropertiesEXT;

            //=== VK_EXT_line_rasterization ===
            loader.vkCmdSetLineStippleEXT = &vkCmdSetLineStippleEXT;
            if (!vkCmdSetLineStipple) loader.vkCmdSetLineStipple = loader.vkCmdSetLineStippleEXT;

            //=== VK_EXT_host_query_reset ===
            loader.vkResetQueryPoolEXT = &vkResetQueryPoolEXT;
            if (!vkResetQueryPool) loader.vkResetQueryPool = loader.vkResetQueryPoolEXT;

            //=== VK_EXT_extended_dynamic_state ===
            loader.vkCmdSetCullModeEXT = &vkCmdSetCullModeEXT;
            if (!vkCmdSetCullMode) loader.vkCmdSetCullMode = loader.vkCmdSetCullModeEXT;
            loader.vkCmdSetFrontFaceEXT = &vkCmdSetFrontFaceEXT;
            if (!vkCmdSetFrontFace) loader.vkCmdSetFrontFace = loader.vkCmdSetFrontFaceEXT;
            loader.vkCmdSetPrimitiveTopologyEXT = &vkCmdSetPrimitiveTopologyEXT;
            if (!vkCmdSetPrimitiveTopology) loader.vkCmdSetPrimitiveTopology = loader.vkCmdSetPrimitiveTopologyEXT;
            loader.vkCmdSetViewportWithCountEXT = &vkCmdSetViewportWithCountEXT;
            if (!vkCmdSetViewportWithCount) loader.vkCmdSetViewportWithCount = loader.vkCmdSetViewportWithCountEXT;
            loader.vkCmdSetScissorWithCountEXT = &vkCmdSetScissorWithCountEXT;
            if (!vkCmdSetScissorWithCount) loader.vkCmdSetScissorWithCount = loader.vkCmdSetScissorWithCountEXT;
            loader.vkCmdBindVertexBuffers2EXT = &vkCmdBindVertexBuffers2EXT;
            if (!vkCmdBindVertexBuffers2) loader.vkCmdBindVertexBuffers2 = loader.vkCmdBindVertexBuffers2EXT;
            loader.vkCmdSetDepthTestEnableEXT = &vkCmdSetDepthTestEnableEXT;
            if (!vkCmdSetDepthTestEnable) loader.vkCmdSetDepthTestEnable = loader.vkCmdSetDepthTestEnableEXT;
            loader.vkCmdSetDepthWriteEnableEXT = &vkCmdSetDepthWriteEnableEXT;
            if (!vkCmdSetDepthWriteEnable) loader.vkCmdSetDepthWriteEnable = loader.vkCmdSetDepthWriteEnableEXT;
            loader.vkCmdSetDepthCompareOpEXT = &vkCmdSetDepthCompareOpEXT;
            if (!vkCmdSetDepthCompareOp) loader.vkCmdSetDepthCompareOp = loader.vkCmdSetDepthCompareOpEXT;
            loader.vkCmdSetDepthBoundsTestEnableEXT = &vkCmdSetDepthBoundsTestEnableEXT;
            if (!vkCmdSetDepthBoundsTestEnable)
                loader.vkCmdSetDepthBoundsTestEnable = loader.vkCmdSetDepthBoundsTestEnableEXT;
            loader.vkCmdSetStencilTestEnableEXT = &vkCmdSetStencilTestEnableEXT;
            if (!vkCmdSetStencilTestEnable) loader.vkCmdSetStencilTestEnable = loader.vkCmdSetStencilTestEnableEXT;
            loader.vkCmdSetStencilOpEXT = &vkCmdSetStencilOpEXT;
            if (!vkCmdSetStencilOp) loader.vkCmdSetStencilOp = loader.vkCmdSetStencilOpEXT;

            //=== VK_EXT_host_image_copy ===
            loader.vkCopyMemoryToImageEXT = &vkCopyMemoryToImageEXT;
            if (!vkCopyMemoryToImage) loader.vkCopyMemoryToImage = loader.vkCopyMemoryToImageEXT;
            loader.vkCopyImageToMemoryEXT = &vkCopyImageToMemoryEXT;
            if (!vkCopyImageToMemory) loader.vkCopyImageToMemory = loader.vkCopyImageToMemoryEXT;
            loader.vkCopyImageToImageEXT = &vkCopyImageToImageEXT;
            if (!vkCopyImageToImage) loader.vkCopyImageToImage = loader.vkCopyImageToImageEXT;
            loader.vkTransitionImageLayoutEXT = &vkTransitionImageLayoutEXT;
            if (!vkTransitionImageLayout) loader.vkTransitionImageLayout = loader.vkTransitionImageLayoutEXT;
            loader.vkGetImageSubresourceLayout2EXT = &vkGetImageSubresourceLayout2EXT;
            if (!vkGetImageSubresourceLayout2)
                loader.vkGetImageSubresourceLayout2 = loader.vkGetImageSubresourceLayout2EXT;

            //=== VK_EXT_depth_bias_control ===
            loader.vkCmdSetDepthBias2EXT = &vkCmdSetDepthBias2EXT;

            //=== VK_EXT_acquire_drm_display ===
            loader.vkAcquireDrmDisplayEXT = &vkAcquireDrmDisplayEXT;
            loader.vkGetDrmDisplayEXT = &vkGetDrmDisplayEXT;

            //=== VK_EXT_descriptor_buffer ===
            loader.vkGetDescriptorSetLayoutSizeEXT = &vkGetDescriptorSetLayoutSizeEXT;
            loader.vkGetDescriptorSetLayoutBindingOffsetEXT = &vkGetDescriptorSetLayoutBindingOffsetEXT;
            loader.vkGetDescriptorEXT = &vkGetDescriptorEXT;
            loader.vkCmdBindDescriptorBuffersEXT = &vkCmdBindDescriptorBuffersEXT;
            loader.vkCmdSetDescriptorBufferOffsetsEXT = &vkCmdSetDescriptorBufferOffsetsEXT;
            loader.vkCmdBindDescriptorBufferEmbeddedSamplersEXT = &vkCmdBindDescriptorBufferEmbeddedSamplersEXT;
            loader.vkGetBufferOpaqueCaptureDescriptorDataEXT = &vkGetBufferOpaqueCaptureDescriptorDataEXT;
            loader.vkGetImageOpaqueCaptureDescriptorDataEXT = &vkGetImageOpaqueCaptureDescriptorDataEXT;
            loader.vkGetImageViewOpaqueCaptureDescriptorDataEXT = &vkGetImageViewOpaqueCaptureDescriptorDataEXT;
            loader.vkGetSamplerOpaqueCaptureDescriptorDataEXT = &vkGetSamplerOpaqueCaptureDescriptorDataEXT;
            loader.vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
                &vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT;

            //=== VK_EXT_device_fault ===
            loader.vkGetDeviceFaultInfoEXT = &vkGetDeviceFaultInfoEXT;

            //=== VK_EXT_multi_draw ===
            loader.vkCmdDrawMultiEXT = &vkCmdDrawMultiEXT;
            loader.vkCmdDrawMultiIndexedEXT = &vkCmdDrawMultiIndexedEXT;

            //=== VK_EXT_opacity_micromap ===
            loader.vkCreateMicromapEXT = &vkCreateMicromapEXT;
            loader.vkDestroyMicromapEXT = &vkDestroyMicromapEXT;
            loader.vkCmdBuildMicromapsEXT = &vkCmdBuildMicromapsEXT;
            loader.vkBuildMicromapsEXT = &vkBuildMicromapsEXT;
            loader.vkCopyMicromapEXT = &vkCopyMicromapEXT;
            loader.vkCopyMicromapToMemoryEXT = &vkCopyMicromapToMemoryEXT;
            loader.vkCopyMemoryToMicromapEXT = &vkCopyMemoryToMicromapEXT;
            loader.vkWriteMicromapsPropertiesEXT = &vkWriteMicromapsPropertiesEXT;
            loader.vkCmdCopyMicromapEXT = &vkCmdCopyMicromapEXT;
            loader.vkCmdCopyMicromapToMemoryEXT = &vkCmdCopyMicromapToMemoryEXT;
            loader.vkCmdCopyMemoryToMicromapEXT = &vkCmdCopyMemoryToMicromapEXT;
            loader.vkCmdWriteMicromapsPropertiesEXT = &vkCmdWriteMicromapsPropertiesEXT;
            loader.vkGetDeviceMicromapCompatibilityEXT = &vkGetDeviceMicromapCompatibilityEXT;
            loader.vkGetMicromapBuildSizesEXT = &vkGetMicromapBuildSizesEXT;

            //=== VK_EXT_pageable_device_local_memory ===
            loader.vkSetDeviceMemoryPriorityEXT = &vkSetDeviceMemoryPriorityEXT;

            //=== VK_EXT_extended_dynamic_state3 ===
            loader.vkCmdSetDepthClampEnableEXT = &vkCmdSetDepthClampEnableEXT;
            loader.vkCmdSetPolygonModeEXT = &vkCmdSetPolygonModeEXT;
            loader.vkCmdSetRasterizationSamplesEXT = &vkCmdSetRasterizationSamplesEXT;
            loader.vkCmdSetSampleMaskEXT = &vkCmdSetSampleMaskEXT;
            loader.vkCmdSetAlphaToCoverageEnableEXT = &vkCmdSetAlphaToCoverageEnableEXT;
            loader.vkCmdSetAlphaToOneEnableEXT = &vkCmdSetAlphaToOneEnableEXT;
            loader.vkCmdSetLogicOpEnableEXT = &vkCmdSetLogicOpEnableEXT;
            loader.vkCmdSetColorBlendEnableEXT = &vkCmdSetColorBlendEnableEXT;
            loader.vkCmdSetColorBlendEquationEXT = &vkCmdSetColorBlendEquationEXT;
            loader.vkCmdSetColorWriteMaskEXT = &vkCmdSetColorWriteMaskEXT;
            loader.vkCmdSetTessellationDomainOriginEXT = &vkCmdSetTessellationDomainOriginEXT;
            loader.vkCmdSetRasterizationStreamEXT = &vkCmdSetRasterizationStreamEXT;
            loader.vkCmdSetConservativeRasterizationModeEXT = &vkCmdSetConservativeRasterizationModeEXT;
            loader.vkCmdSetExtraPrimitiveOverestimationSizeEXT = &vkCmdSetExtraPrimitiveOverestimationSizeEXT;
            loader.vkCmdSetDepthClipEnableEXT = &vkCmdSetDepthClipEnableEXT;
            loader.vkCmdSetSampleLocationsEnableEXT = &vkCmdSetSampleLocationsEnableEXT;
            loader.vkCmdSetColorBlendAdvancedEXT = &vkCmdSetColorBlendAdvancedEXT;
            loader.vkCmdSetProvokingVertexModeEXT = &vkCmdSetProvokingVertexModeEXT;
            loader.vkCmdSetLineRasterizationModeEXT = &vkCmdSetLineRasterizationModeEXT;
            loader.vkCmdSetLineStippleEnableEXT = &vkCmdSetLineStippleEnableEXT;
            loader.vkCmdSetDepthClipNegativeOneToOneEXT = &vkCmdSetDepthClipNegativeOneToOneEXT;
            loader.vkCmdSetViewportWScalingEnableNV = &vkCmdSetViewportWScalingEnableNV;
            loader.vkCmdSetViewportSwizzleNV = &vkCmdSetViewportSwizzleNV;
            loader.vkCmdSetCoverageToColorEnableNV = &vkCmdSetCoverageToColorEnableNV;
            loader.vkCmdSetCoverageToColorLocationNV = &vkCmdSetCoverageToColorLocationNV;
            loader.vkCmdSetCoverageModulationModeNV = &vkCmdSetCoverageModulationModeNV;
            loader.vkCmdSetCoverageModulationTableEnableNV = &vkCmdSetCoverageModulationTableEnableNV;
            loader.vkCmdSetCoverageModulationTableNV = &vkCmdSetCoverageModulationTableNV;
            loader.vkCmdSetShadingRateImageEnableNV = &vkCmdSetShadingRateImageEnableNV;
            loader.vkCmdSetRepresentativeFragmentTestEnableNV = &vkCmdSetRepresentativeFragmentTestEnableNV;
            loader.vkCmdSetCoverageReductionModeNV = &vkCmdSetCoverageReductionModeNV;

            //=== VK_EXT_shader_object ===
            loader.vkCreateShadersEXT = &vkCreateShadersEXT;
            loader.vkDestroyShaderEXT = &vkDestroyShaderEXT;
            loader.vkGetShaderBinaryDataEXT = &vkGetShaderBinaryDataEXT;
            loader.vkCmdBindShadersEXT = &vkCmdBindShadersEXT;
            loader.vkCmdSetDepthClampRangeEXT = &vkCmdSetDepthClampRangeEXT;

            //=== VK_EXT_attachment_feedback_loop_dynamic_state ===
            loader.vkCmdSetAttachmentFeedbackLoopEnableEXT = &vkCmdSetAttachmentFeedbackLoopEnableEXT;

            //=== VK_EXT_memory_decompression ===
            loader.vkCmdDecompressMemoryEXT = &vkCmdDecompressMemoryEXT;
            loader.vkCmdDecompressMemoryIndirectCountEXT = &vkCmdDecompressMemoryIndirectCountEXT;

            //=== VK_EXT_device_generated_commands ===
            loader.vkGetGeneratedCommandsMemoryRequirementsEXT = &vkGetGeneratedCommandsMemoryRequirementsEXT;
            loader.vkCmdPreprocessGeneratedCommandsEXT = &vkCmdPreprocessGeneratedCommandsEXT;
            loader.vkCmdExecuteGeneratedCommandsEXT = &vkCmdExecuteGeneratedCommandsEXT;
            loader.vkCreateIndirectCommandsLayoutEXT = &vkCreateIndirectCommandsLayoutEXT;
            loader.vkDestroyIndirectCommandsLayoutEXT = &vkDestroyIndirectCommandsLayoutEXT;
            loader.vkCreateIndirectExecutionSetEXT = &vkCreateIndirectExecutionSetEXT;
            loader.vkDestroyIndirectExecutionSetEXT = &vkDestroyIndirectExecutionSetEXT;
            loader.vkUpdateIndirectExecutionSetPipelineEXT = &vkUpdateIndirectExecutionSetPipelineEXT;
            loader.vkUpdateIndirectExecutionSetShaderEXT = &vkUpdateIndirectExecutionSetShaderEXT;

            //=== VK_EXT_fragment_density_map_offset ===
            loader.vkCmdEndRendering2EXT = &vkCmdEndRendering2EXT;
            if (!vkCmdEndRendering2KHR) loader.vkCmdEndRendering2KHR = loader.vkCmdEndRendering2EXT;

            //=== VK_EXT_custom_resolve ===
            loader.vkCmdBeginCustomResolveEXT = &vkCmdBeginCustomResolveEXT;

            //=== VK_NV_compute_occupancy_priority ===
            loader.vkCmdSetComputeOccupancyPriorityNV = &vkCmdSetComputeOccupancyPriorityNV;
#endif
#endif
        }

        static void create_command_pool(device &device, queue_family_info &queue, size_t primary, size_t secondary)
        {
            if (!queue.family_id.has_value()) return;

            vk::CommandPoolCreateInfo create_info;
            create_info
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient |
                          vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                .setQueueFamilyIndex(queue.family_id.value());

            queue.pool.vk_pool = device.vk_device.createCommandPool(create_info, nullptr, device.loader);

            queue.pool.primary.allocator.device = &device.vk_device;
            queue.pool.primary.allocator.loader = &device.loader;
            queue.pool.primary.allocator.command_pool = &queue.pool.vk_pool;
            queue.pool.primary.allocate(primary);

            queue.pool.secondary.allocator.device = &device.vk_device;
            queue.pool.secondary.allocator.loader = &device.loader;
            queue.pool.secondary.allocator.command_pool = &queue.pool.vk_pool;
            queue.pool.secondary.allocate(secondary);
        }

        static bool create_allocator(device &device)
        {
            VmaVulkanFunctions vma_functions = {};
            vma_functions.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                device.instance.getProcAddr("vkGetInstanceProcAddr", device.loader));
            vma_functions.vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
                device.vk_device.getProcAddr("vkGetDeviceProcAddr", device.loader));

            VmaAllocatorCreateInfo allocator_info = {};
            allocator_info.physicalDevice = device.physical_device;
            allocator_info.device = device.vk_device;
            allocator_info.instance = device.instance;
            allocator_info.pVulkanFunctions = &vma_functions;
            allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;
            allocator_info.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
            return vmaCreateAllocator(&allocator_info, &device.allocator) == VK_SUCCESS;
        }
    } // namespace

    void initialize_adopted_device(device &device, const adopted_device_create_info &create_info)
    {
        if (!create_info.instance || !create_info.physical_device || !create_info.vk_device)
            throw acul::runtime_error("adopt_device: required Vulkan handles are not set");

        device.instance = create_info.instance;
        device.physical_device = create_info.physical_device;
        device.vk_device = create_info.vk_device;

        init_dispatch_loader(device);

        if (!create_info.runtime_data) throw acul::runtime_error("adopt_device: runtime_data is required");
        device.rd = create_info.runtime_data;

        auto &queues = device.rd->queues;
        queues.graphics.family_id = create_info.graphics_family_id;
        queues.graphics.vk_queue = create_info.graphics_queue;
        queues.compute.family_id = create_info.compute_family_id;
        queues.compute.vk_queue = create_info.compute_queue;
        if (create_info.has_present_queue)
        {
            queues.present.family_id = create_info.present_family_id;
            queues.present.vk_queue = create_info.present_queue;
        }

        device.rd->properties2 = device.physical_device.getProperties2(device.loader);
        device.rd->memory_properties = device.physical_device.getMemoryProperties(device.loader);

        if (create_info.create_command_pools)
        {
            create_command_pool(device, queues.graphics, create_info.graphics_primary_buffers,
                                create_info.graphics_secondary_buffers);
            create_command_pool(device, queues.compute, create_info.compute_primary_buffers,
                                create_info.compute_secondary_buffers);
        }

        if (create_info.create_fence_pool)
        {
            auto &fence_pool = device.rd->fence_pool;
            fence_pool.allocator.device = &device.vk_device;
            fence_pool.allocator.loader = &device.loader;
            fence_pool.allocate(create_info.fence_pool_size);
        }

        device.allocator = create_info.allocator;
    }

    void destroy_adopted_device(device &device)
    {
        if (!device.rd) return;

        auto &queues = device.rd->queues;
        if (queues.graphics.pool.vk_pool || queues.compute.pool.vk_pool)
            queues.destroy(device.vk_device, device.loader);

        auto &fence_pool = device.rd->fence_pool;
        if (fence_pool.allocator.device) fence_pool.destroy();
    }

    bool create_adopted_allocator(device &device)
    {
        if (device.allocator) return true;
        return create_allocator(device);
    }

    void destroy_adopted_allocator(device &device)
    {
        if (!device.allocator) return;
        vmaDestroyAllocator(device.allocator);
        device.allocator = nullptr;
    }
} // namespace agrb
