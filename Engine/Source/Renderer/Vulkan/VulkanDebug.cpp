#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
namespace v3d
{
namespace renderer
{
namespace vk
{

std::string ErrorString(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

const std::vector<const c8*> VulkanDebug::s_validationLayerNames =
{
#ifdef PLATFORM_WINDOWS
    // This is a meta layer that enables all of the standard
    // validation layers in the correct order :
    // threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
    "VK_LAYER_LUNARG_standard_validation"
#else
    // Parse individual layers in specific order and as they're available, include them
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_parameter_validation", // should be early in the list
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_device_limits",
    "VK_LAYER_LUNARG_image",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_GOOGLE_unique_objects", // should be late in the list, gets data early from the driver
#endif
};

VkDebugUtilsMessengerEXT VulkanDebug::s_messeger = VK_NULL_HANDLE;

bool VulkanDebug::createDebugUtilsMesseger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severityFlag, VkDebugUtilsMessageTypeFlagsEXT flags, PFN_vkDebugUtilsMessengerCallbackEXT callback, void * userData)
{
    if (s_messeger)
    {
        LOG_WARNING("VulkanDebug::createDebugUtilsMessagerCallback: already exist");
        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = nullptr;
    debugUtilsMessengerCreateInfo.flags = 0;
    debugUtilsMessengerCreateInfo.messageSeverity = severityFlag;
    debugUtilsMessengerCreateInfo.messageType = flags;
    debugUtilsMessengerCreateInfo.pfnUserCallback = (callback) ? callback : VulkanDebug::defaultDebugUtilsMessegerCallback;
    debugUtilsMessengerCreateInfo.pUserData = userData;

    VkResult result = VulkanWrapper::CreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, VULKAN_ALLOCATOR, &s_messeger);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::createDebugUtilsMessagerCallback: vkCreateDebugUtilsMessengerEXT error %s", ErrorString(result).c_str());
        return false;
    }
    return true;
}

void VulkanDebug::destroyDebugUtilsMesseger(VkInstance instance)
{
    if (s_messeger)
    {
        VulkanWrapper::DestroyDebugUtilsMessengerEXT(instance, s_messeger, VULKAN_ALLOCATOR);
        s_messeger = VK_NULL_HANDLE;
    }
}

VkBool32 VulkanDebug::defaultDebugUtilsMessegerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData)
{
    switch (messageType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARNING("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LOG_INFO("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            LOG_DEBUG("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARNING("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LOG_INFO("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            LOG_DEBUG("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }
        return VK_TRUE;
    }

    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    {
        LOG_WARNING("Performance[Servity level %u][%s]Code %d: %s", messageSeverity, pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        return VK_FALSE;
    }

    }
    return VK_FALSE;
}

bool VulkanDebug::checkInstanceLayerIsSupported(const c8* layerName)
{
    u32 instanceLayerPropertyCount = 0;
    VkResult result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::checkInstanceLayerIsSupported: vkEnumerateInstanceLayerProperties count error %s", vk::ErrorString(result).c_str());
        return false;
    }

    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    if (!instanceLayerProperties.empty())
    {
        result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanDebug::checkInstanceLayerIsSupported: vkEnumerateInstanceLayerProperties list error %s", vk::ErrorString(result).c_str());
            return false;
        }

        for (auto iter = instanceLayerProperties.cbegin(); iter < instanceLayerProperties.cend(); ++iter)
        {
            if (!strcmp((*iter).layerName, layerName))
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

bool VulkanDebug::checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName)
{
    u32 deviceLayerPropertyCount = 0;
    VkResult result = VulkanWrapper::EnumerateDeviceLayerProperties(device, &deviceLayerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::checkDeviceLayerIsSupported: vkEnumerateDeviceLayerProperties count error %s", vk::ErrorString(result).c_str());
        return false;
    }

    std::vector<VkLayerProperties> deviceLayerProperties(deviceLayerPropertyCount);
    if (!deviceLayerProperties.empty())
    {
        result = VulkanWrapper::EnumerateDeviceLayerProperties(device, &deviceLayerPropertyCount, deviceLayerProperties.data());
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanDebug::checkDeviceLayerIsSupported: vkEnumerateDeviceLayerProperties list error %s", vk::ErrorString(result).c_str());
            return false;
        }

        for (auto iter = deviceLayerProperties.cbegin(); iter < deviceLayerProperties.cend(); ++iter)
        {
            if (!strcmp((*iter).layerName, layerName))
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
