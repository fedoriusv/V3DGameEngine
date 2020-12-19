#pragma once
#include "Common.h"

#ifdef VULKAN_RENDER

#if defined(PLATFORM_WINDOWS)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(PLATFORM_LINUX)
#   define VK_USE_PLATFORM_XLIB_KHR
#elif defined(PLATFORM_ANDROID)
#   define VK_NO_PROTOTYPES
#   define VK_USE_PLATFORM_ANDROID_KHR
#endif

#include <vulkan/vulkan.h>

#if defined(PLATFORM_ANDROID)
#   include "Platform/Android/AndroidVulkanExtension.h"
#endif

#ifdef VK_NO_PROTOTYPES
    //Base functions
#   define ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(Macro) \
    Macro(vkCreateInstance) \
    Macro(vkGetInstanceProcAddr) \
    Macro(vkGetDeviceProcAddr) \
    Macro(vkEnumerateInstanceExtensionProperties) \
    Macro(vkEnumerateInstanceLayerProperties) \
    Macro(vkEnumerateInstanceVersion)

    //Core functions
#   define ENUM_VK_ENTRYPOINTS_CORE_FUNCTIONS(Macro) \
    Macro(vkDestroyInstance) \
    Macro(vkEnumeratePhysicalDevices) \
    Macro(vkGetPhysicalDeviceFeatures) \
    Macro(vkGetPhysicalDeviceFormatProperties) \
    Macro(vkGetPhysicalDeviceImageFormatProperties) \
    Macro(vkGetPhysicalDeviceProperties) \
    Macro(vkGetPhysicalDeviceQueueFamilyProperties) \
    Macro(vkGetPhysicalDeviceMemoryProperties) \
    Macro(vkCreateDevice) \
    Macro(vkDestroyDevice) \
    Macro(vkEnumerateDeviceExtensionProperties) \
    Macro(vkEnumerateDeviceLayerProperties) \
    Macro(vkGetDeviceQueue) \
    Macro(vkQueueSubmit) \
    Macro(vkQueueWaitIdle) \
    Macro(vkDeviceWaitIdle) \
    Macro(vkAllocateMemory) \
    Macro(vkFreeMemory) \
    Macro(vkMapMemory) \
    Macro(vkUnmapMemory) \
    Macro(vkFlushMappedMemoryRanges) \
    Macro(vkInvalidateMappedMemoryRanges) \
    Macro(vkGetDeviceMemoryCommitment) \
    Macro(vkBindBufferMemory) \
    Macro(vkBindImageMemory) \
    Macro(vkGetBufferMemoryRequirements) \
    Macro(vkGetImageMemoryRequirements) \
    Macro(vkGetImageSparseMemoryRequirements) \
    Macro(vkGetPhysicalDeviceSparseImageFormatProperties) \
    Macro(vkQueueBindSparse) \
    Macro(vkCreateFence) \
    Macro(vkDestroyFence) \
    Macro(vkResetFences) \
    Macro(vkGetFenceStatus) \
    Macro(vkWaitForFences) \
    Macro(vkCreateSemaphore) \
    Macro(vkDestroySemaphore) \
    Macro(vkCreateEvent) \
    Macro(vkDestroyEvent) \
    Macro(vkGetEventStatus) \
    Macro(vkSetEvent) \
    Macro(vkResetEvent) \
    Macro(vkCreateQueryPool) \
    Macro(vkDestroyQueryPool) \
    Macro(vkGetQueryPoolResults) \
    Macro(vkCreateBuffer) \
    Macro(vkDestroyBuffer) \
    Macro(vkCreateBufferView) \
    Macro(vkDestroyBufferView) \
    Macro(vkCreateImage) \
    Macro(vkDestroyImage) \
    Macro(vkGetImageSubresourceLayout) \
    Macro(vkCreateImageView) \
    Macro(vkDestroyImageView) \
    Macro(vkCreateShaderModule) \
    Macro(vkDestroyShaderModule) \
    Macro(vkCreatePipelineCache) \
    Macro(vkDestroyPipelineCache) \
    Macro(vkGetPipelineCacheData) \
    Macro(vkMergePipelineCaches) \
    Macro(vkCreateGraphicsPipelines) \
    Macro(vkCreateComputePipelines) \
    Macro(vkDestroyPipeline) \
    Macro(vkCreatePipelineLayout) \
    Macro(vkDestroyPipelineLayout) \
    Macro(vkCreateSampler) \
    Macro(vkDestroySampler) \
    Macro(vkCreateDescriptorSetLayout) \
    Macro(vkDestroyDescriptorSetLayout) \
    Macro(vkCreateDescriptorPool) \
    Macro(vkDestroyDescriptorPool) \
    Macro(vkResetDescriptorPool) \
    Macro(vkAllocateDescriptorSets) \
    Macro(vkFreeDescriptorSets) \
    Macro(vkUpdateDescriptorSets) \
    Macro(vkCreateFramebuffer) \
    Macro(vkDestroyFramebuffer) \
    Macro(vkCreateRenderPass) \
    Macro(vkDestroyRenderPass) \
    Macro(vkGetRenderAreaGranularity) \
    Macro(vkCreateCommandPool) \
    Macro(vkDestroyCommandPool) \
    Macro(vkResetCommandPool) \
    Macro(vkAllocateCommandBuffers) \
    Macro(vkFreeCommandBuffers) \
    Macro(vkBeginCommandBuffer) \
    Macro(vkEndCommandBuffer) \
    Macro(vkResetCommandBuffer) \
    Macro(vkCmdBindPipeline) \
    Macro(vkCmdSetViewport) \
    Macro(vkCmdSetScissor) \
    Macro(vkCmdSetLineWidth) \
    Macro(vkCmdSetDepthBias) \
    Macro(vkCmdSetBlendConstants) \
    Macro(vkCmdSetDepthBounds) \
    Macro(vkCmdSetStencilCompareMask) \
    Macro(vkCmdSetStencilWriteMask) \
    Macro(vkCmdSetStencilReference) \
    Macro(vkCmdBindDescriptorSets) \
    Macro(vkCmdBindIndexBuffer) \
    Macro(vkCmdBindVertexBuffers) \
    Macro(vkCmdDraw) \
    Macro(vkCmdDrawIndexed) \
    Macro(vkCmdDrawIndirect) \
    Macro(vkCmdDrawIndexedIndirect) \
    Macro(vkCmdDispatch) \
    Macro(vkCmdDispatchIndirect) \
    Macro(vkCmdCopyBuffer) \
    Macro(vkCmdCopyImage) \
    Macro(vkCmdBlitImage) \
    Macro(vkCmdCopyBufferToImage) \
    Macro(vkCmdCopyImageToBuffer) \
    Macro(vkCmdUpdateBuffer) \
    Macro(vkCmdFillBuffer) \
    Macro(vkCmdClearColorImage) \
    Macro(vkCmdClearDepthStencilImage) \
    Macro(vkCmdClearAttachments) \
    Macro(vkCmdResolveImage) \
    Macro(vkCmdSetEvent) \
    Macro(vkCmdResetEvent) \
    Macro(vkCmdWaitEvents) \
    Macro(vkCmdPipelineBarrier) \
    Macro(vkCmdBeginQuery) \
    Macro(vkCmdEndQuery) \
    Macro(vkCmdResetQueryPool) \
    Macro(vkCmdWriteTimestamp) \
    Macro(vkCmdCopyQueryPoolResults) \
    Macro(vkCmdPushConstants) \
    Macro(vkCmdBeginRenderPass) \
    Macro(vkCmdNextSubpass) \
    Macro(vkCmdEndRenderPass) \
    Macro(vkCmdExecuteCommands)

    //Vulkan core
#   if (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_0)
#       define ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(Macro) \
        Macro(vkBindBufferMemory2KHR) \
        Macro(vkBindImageMemory2KHR) \
        Macro(vkGetDeviceGroupPeerMemoryFeaturesKHR) \
        Macro(vkCmdSetDeviceMaskKHR) \
        Macro(vkCmdDispatchBaseKHR) \
        Macro(vkEnumeratePhysicalDeviceGroupsKHR) \
        Macro(vkGetImageMemoryRequirements2KHR) \
        Macro(vkGetBufferMemoryRequirements2KHR) \
        Macro(vkGetImageSparseMemoryRequirements2KHR) \
        Macro(vkGetPhysicalDeviceFeatures2KHR) \
        Macro(vkGetPhysicalDeviceProperties2KHR) \
        Macro(vkGetPhysicalDeviceFormatProperties2KHR) \
        Macro(vkGetPhysicalDeviceImageFormatProperties2KHR) \
        Macro(vkGetPhysicalDeviceQueueFamilyProperties2KHR) \
        Macro(vkGetPhysicalDeviceMemoryProperties2KHR) \
        Macro(vkGetPhysicalDeviceSparseImageFormatProperties2KHR) \
        Macro(vkTrimCommandPoolKHR) \
        Macro(vkGetDeviceQueue2) \
        Macro(vkCreateSamplerYcbcrConversionKHR) \
        Macro(vkDestroySamplerYcbcrConversionKHR) \
        Macro(vkCreateDescriptorUpdateTemplateKHR) \
        Macro(vkDestroyDescriptorUpdateTemplateKHR) \
        Macro(vkUpdateDescriptorSetWithTemplateKHR) \
        Macro(vkGetPhysicalDeviceExternalBufferPropertiesKHR) \
        Macro(vkGetPhysicalDeviceExternalFencePropertiesKHR) \
        Macro(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR) \
        Macro(vkGetDescriptorSetLayoutSupportKHR)

#       define ENUM_VK_ENTRYPOINTS_CORE_1_2_FUNCTIONS(Macro) \
        Macro(vkCmdDrawIndirectCountKHR) \
        Macro(vkCmdDrawIndexedIndirectCountKHR) \
        Macro(vkCreateRenderPass2KHR) \
        Macro(vkCmdBeginRenderPass2KHR) \
        Macro(vkCmdNextSubpass2KHR) \
        Macro(vkCmdEndRenderPass2KHR) \
        Macro(vkResetQueryPool) \
        Macro(vkGetSemaphoreCounterValueKHR) \
        Macro(vkWaitSemaphoresKHR) \
        Macro(vkSignalSemaphoreKHR) \
        Macro(vkGetBufferDeviceAddressKHR) \
        Macro(vkGetBufferOpaqueCaptureAddressKHR) \
        Macro(vkGetDeviceMemoryOpaqueCaptureAddressKHR)

#   elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_1)
#       define ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(Macro) \
        Macro(vkBindBufferMemory2) \
        Macro(vkBindImageMemory2) \
        Macro(vkGetDeviceGroupPeerMemoryFeatures) \
        Macro(vkCmdSetDeviceMask) \
        Macro(vkCmdDispatchBase) \
        Macro(vkEnumeratePhysicalDeviceGroups) \
        Macro(vkGetImageMemoryRequirements2) \
        Macro(vkGetBufferMemoryRequirements2) \
        Macro(vkGetImageSparseMemoryRequirements2) \
        Macro(vkGetPhysicalDeviceFeatures2) \
        Macro(vkGetPhysicalDeviceProperties2) \
        Macro(vkGetPhysicalDeviceFormatProperties2) \
        Macro(vkGetPhysicalDeviceImageFormatProperties2) \
        Macro(vkGetPhysicalDeviceQueueFamilyProperties2) \
        Macro(vkGetPhysicalDeviceMemoryProperties2) \
        Macro(vkGetPhysicalDeviceSparseImageFormatProperties2) \
        Macro(vkTrimCommandPool) \
        Macro(vkGetDeviceQueue2) \
        Macro(vkCreateSamplerYcbcrConversion) \
        Macro(vkDestroySamplerYcbcrConversion) \
        Macro(vkCreateDescriptorUpdateTemplate) \
        Macro(vkDestroyDescriptorUpdateTemplate) \
        Macro(vkUpdateDescriptorSetWithTemplate) \
        Macro(vkGetPhysicalDeviceExternalBufferProperties) \
        Macro(vkGetPhysicalDeviceExternalFenceProperties) \
        Macro(vkGetPhysicalDeviceExternalSemaphoreProperties) \
        Macro(vkGetDescriptorSetLayoutSupport)

#       define ENUM_VK_ENTRYPOINTS_CORE_1_2_FUNCTIONS(Macro) \
        Macro(vkCmdDrawIndirectCountKHR) \
        Macro(vkCmdDrawIndexedIndirectCountKHR) \
        Macro(vkCreateRenderPass2KHR) \
        Macro(vkCmdBeginRenderPass2KHR) \
        Macro(vkCmdNextSubpass2KHR) \
        Macro(vkCmdEndRenderPass2KHR) \
        Macro(vkResetQueryPool) \
        Macro(vkGetSemaphoreCounterValueKHR) \
        Macro(vkWaitSemaphoresKHR) \
        Macro(vkSignalSemaphoreKHR) \
        Macro(vkGetBufferDeviceAddressKHR) \
        Macro(vkGetBufferOpaqueCaptureAddressKHR) \
        Macro(vkGetDeviceMemoryOpaqueCaptureAddressKHR)

#   else //VULKAN_VERSION_1_2
#       define ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(Macro) \
        Macro(vkBindBufferMemory2) \
        Macro(vkBindImageMemory2) \
        Macro(vkGetDeviceGroupPeerMemoryFeatures) \
        Macro(vkCmdSetDeviceMask) \
        Macro(vkCmdDispatchBase) \
        Macro(vkEnumeratePhysicalDeviceGroups) \
        Macro(vkGetImageMemoryRequirements2) \
        Macro(vkGetBufferMemoryRequirements2) \
        Macro(vkGetImageSparseMemoryRequirements2) \
        Macro(vkGetPhysicalDeviceFeatures2) \
        Macro(vkGetPhysicalDeviceProperties2) \
        Macro(vkGetPhysicalDeviceFormatProperties2) \
        Macro(vkGetPhysicalDeviceImageFormatProperties2) \
        Macro(vkGetPhysicalDeviceQueueFamilyProperties2) \
        Macro(vkGetPhysicalDeviceMemoryProperties2) \
        Macro(vkGetPhysicalDeviceSparseImageFormatProperties2) \
        Macro(vkTrimCommandPool) \
        Macro(vkGetDeviceQueue2) \
        Macro(vkCreateSamplerYcbcrConversion) \
        Macro(vkDestroySamplerYcbcrConversion) \
        Macro(vkCreateDescriptorUpdateTemplate) \
        Macro(vkDestroyDescriptorUpdateTemplate) \
        Macro(vkUpdateDescriptorSetWithTemplate) \
        Macro(vkGetPhysicalDeviceExternalBufferProperties) \
        Macro(vkGetPhysicalDeviceExternalFenceProperties) \
        Macro(vkGetPhysicalDeviceExternalSemaphoreProperties) \
        Macro(vkGetDescriptorSetLayoutSupport)

#       define ENUM_VK_ENTRYPOINTS_CORE_1_2_FUNCTIONS(Macro) \
        Macro(vkCmdDrawIndirectCount) \
        Macro(vkCmdDrawIndexedIndirectCount) \
        Macro(vkCreateRenderPass2) \
        Macro(vkCmdBeginRenderPass2) \
        Macro(vkCmdNextSubpass2) \
        Macro(vkCmdEndRenderPass2) \
        Macro(vkResetQueryPool) \
        Macro(vkGetSemaphoreCounterValue) \
        Macro(vkWaitSemaphores) \
        Macro(vkSignalSemaphore) \
        Macro(vkGetBufferDeviceAddress) \
        Macro(vkGetBufferOpaqueCaptureAddress) \
        Macro(vkGetDeviceMemoryOpaqueCaptureAddress)
#   endif //VULKAN_VERSION

    //Surface
#   ifdef VK_KHR_surface
#   define ENUM_VK_ENTRYPOINTS_SURFACE_FUNCTIONS(Macro) \
    Macro(vkDestroySurfaceKHR) \
    Macro(vkGetPhysicalDeviceSurfaceSupportKHR) \
    Macro(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    Macro(vkGetPhysicalDeviceSurfaceFormatsKHR) \
    Macro(vkGetPhysicalDeviceSurfacePresentModesKHR)
#   endif //VK_KHR_surface

    //Swapchain
#   ifdef VK_KHR_swapchain
#   define ENUM_VK_ENTRYPOINTS_SWAPCHAIN_FUNCTIONS(Macro) \
    Macro(vkCreateSwapchainKHR) \
    Macro(vkDestroySwapchainKHR) \
    Macro(vkGetSwapchainImagesKHR) \
    Macro(vkAcquireNextImageKHR) \
    Macro(vkQueuePresentKHR) \
    Macro(vkGetDeviceGroupPresentCapabilitiesKHR) \
    Macro(vkGetDeviceGroupSurfacePresentModesKHR) \
    Macro(vkGetPhysicalDevicePresentRectanglesKHR) \
    Macro(vkAcquireNextImage2KHR)
#   endif //VK_KHR_swapchain

    //Platform
#   ifdef VK_KHR_win32_surface
#   define ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(Macro) \
    Macro(vkCreateWin32SurfaceKHR)
#   elif VK_KHR_android_surface
#   define ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(Macro) \
    Macro(vkCreateAndroidSurfaceKHR)
#   else
#   define ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(Macro)
#   endif //VK_KHR_*_surface
#else //VK_NO_PROTOTYPES
#   define ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_CORE_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_CORE_1_2_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_SURFACE_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_SWAPCHAIN_FUNCTIONS(Macro)
#   define ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(Macro)
#endif //VK_NO_PROTOTYPES

    //Debug Report
#ifdef VK_EXT_debug_report
#   define ENUM_VK_DEBUG_REPORT_FUNCTIONS(Macro) \
    Macro(vkCreateDebugReportCallbackEXT) \
    Macro(vkDestroyDebugReportCallbackEXT) \
    Macro(vkDebugReportMessageEXT)
#else
#   define ENUM_VK_DEBUG_REPORT_FUNCTIONS(Macro)
#endif //VK_EXT_debug_report

    //Debug Utils
#ifdef VK_EXT_debug_utils
#   define ENUM_VK_DEBUG_UTILS_FUNCTIONS(Macro) \
    Macro(vkSetDebugUtilsObjectNameEXT) \
    Macro(vkSetDebugUtilsObjectTagEXT) \
    Macro(vkQueueBeginDebugUtilsLabelEXT) \
    Macro(vkQueueEndDebugUtilsLabelEXT) \
    Macro(vkQueueInsertDebugUtilsLabelEXT) \
    Macro(vkCmdBeginDebugUtilsLabelEXT) \
    Macro(vkCmdEndDebugUtilsLabelEXT) \
    Macro(vkCmdInsertDebugUtilsLabelEXT) \
    Macro(vkCreateDebugUtilsMessengerEXT) \
    Macro(vkDestroyDebugUtilsMessengerEXT) \
    Macro(vkSubmitDebugUtilsMessageEXT)
#   else
#   define ENUM_VK_DEBUG_UTILS_FUNCTIONS(Macro)
#endif //VK_EXT_debug_utils

#ifdef VK_KHR_pipeline_executable_properties
#   define ENUM_VK_PIPELINE_EXECUTABLE_PROPERTIES_FUNCTIONS(Macro) \
    Macro(vkGetPipelineExecutablePropertiesKHR) \
    Macro(vkGetPipelineExecutableStatisticsKHR) \
    Macro(vkGetPipelineExecutableInternalRepresentationsKHR)
#   else
#   define ENUM_VK_PIPELINE_EXECUTABLE_PROPERTIES_FUNCTIONS(Macro)
#endif // VK_KHR_pipeline_executable_properties

#define ENUM_VK_ALL_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_CORE_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_CORE_1_2_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_SURFACE_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_SWAPCHAIN_FUNCTIONS(Macro) \
    ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(Macro) \
    ENUM_VK_DEBUG_REPORT_FUNCTIONS(Macro) \
    ENUM_VK_DEBUG_UTILS_FUNCTIONS(Macro) \
    ENUM_VK_PIPELINE_EXECUTABLE_PROPERTIES_FUNCTIONS(Macro)

#define DECLARE_VK_FUNCTIONS(Func) extern PFN_##Func Func;

namespace v3d
{
namespace renderer
{
namespace vk
{
    ENUM_VK_ALL_FUNCTIONS(DECLARE_VK_FUNCTIONS);

    bool LoadVulkanLibrary();
    bool LoadVulkanLibrary(VkInstance instance);
    bool LoadVulkanLibrary(VkDevice device);
    void FreeVulkanLibrary();

    #undef DECLARE_VK_FUNCTIONS

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
