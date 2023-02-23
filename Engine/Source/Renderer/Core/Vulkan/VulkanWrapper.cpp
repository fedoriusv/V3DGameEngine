#include "VulkanWrapper.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

#ifdef VK_EXT_debug_report
VkResult VulkanWrapper::CreateDebugReportCallback(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    return vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

void VulkanWrapper::DestroyDebugReportCallback(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void VulkanWrapper::DebugReportMessage(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    vkDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}
#endif //VK_EXT_debug_report

#ifdef VK_EXT_debug_utils
VkResult VulkanWrapper::SetDebugUtilsObjectName(VkDevice device, const VkDebugUtilsObjectNameInfoEXT * pNameInfo) noexcept
{
    return vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

VkResult VulkanWrapper::SetDebugUtilsObjectTag(VkDevice device, const VkDebugUtilsObjectTagInfoEXT * pTagInfo) noexcept
{
    return vkSetDebugUtilsObjectTagEXT(device, pTagInfo);
}

void VulkanWrapper::QueueBeginDebugUtilsLabel(VkQueue queue, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void VulkanWrapper::QueueEndDebugUtilsLabel(VkQueue queue) noexcept
{
    vkQueueEndDebugUtilsLabelEXT(queue);
}

void VulkanWrapper::QueueInsertDebugUtilsLabel(VkQueue queue, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void VulkanWrapper::CmdBeginDebugUtilsLabel(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void VulkanWrapper::CmdEndDebugUtilsLabel(VkCommandBuffer commandBuffer) noexcept
{
    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void VulkanWrapper::CmdInsertDebugUtilsLabel(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VkResult VulkanWrapper::CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugUtilsMessengerEXT * pMessenger) noexcept
{
    return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

void VulkanWrapper::DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void VulkanWrapper::SubmitDebugUtilsMessage(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData) noexcept
{
    vkSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#endif //VK_EXT_debug_utils

#ifdef VK_KHR_pipeline_executable_properties
VkResult VulkanWrapper::GetPipelineExecutableProperties(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) noexcept
{
    return vkGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
}

VkResult VulkanWrapper::GetPipelineExecutableStatistics(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) noexcept
{
    return vkGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
}

VkResult VulkanWrapper::GetPipelineExecutableInternalRepresentations(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) noexcept
{
    return vkGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
#endif //VK_KHR_pipeline_executable_properties

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
