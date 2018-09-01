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

#if VULKAN_VALIDATION_LAYERS_CALLBACK
const bool VulkanDebug::s_enableValidationLayers = true;
#else
const bool VulkanDebug::s_enableValidationLayers = false;
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

#if VULKAN_DEBUG_MARKERS
const bool VulkanDebug::s_enableDebugMarkers = true;
#else
const bool VulkanDebug::s_enableDebugMarkers = false;
#endif //VULKAN_DEBUG_MARKERS

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
    "VK_LAYER_GOOGLE_unique_objects", // should be late in the list, gets data early from the driver
#endif
};

PFN_vkCreateDebugReportCallbackEXT  VulkanDebug::s_vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT VulkanDebug::s_vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

VkDebugReportCallbackEXT            VulkanDebug::s_msgCallback;

VkBool32 VulkanDebug::messageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, u64 srcObject, size_t location, s32 msgCode, c8* layerPrefix, c8* msg, void* userData)
{
    // Error that may result in undefined behaviour
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        LOG_ERROR("[%s]Object: %d Code %d: %s", layerPrefix, srcObject, msgCode, msg);
    };
    // Warnings may hint at unexpected / non-spec API usage
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        LOG_WARNING("[%s]Object: %d Code %d: %s", layerPrefix, srcObject, msgCode, msg);
    };
    // May indicate sub-optimal usage of the API
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        LOG_WARNING("[%s]Object: %d Code %d: %s", layerPrefix, srcObject, msgCode, msg);
    };
    // Informal messages that may become handy during debugging
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        LOG_INFO("[%s]Object: %d Code %d: %s", layerPrefix, srcObject, msgCode, msg);
    }
    // Diagnostic info from the Vulkan loader and layers
    // Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        LOG_DEBUG("[%s]Object: %d Code %d: %s", layerPrefix, srcObject, msgCode, msg);
    }

    // The return value of this callback controls wether the Vulkan call that caused
    // the validation message will be aborted or not
    // We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
    // (and return a VkResult) to abort
    // If you instead want to have calls abort, pass in VK_TRUE and the function will 
    // return VK_ERROR_VALIDATION_FAILED_EXT 
    if (VulkanDebug::s_enableValidationLayers)
    {
        return VK_TRUE;
    }
    else
    {
        return VK_FALSE;
    }
}

void VulkanDebug::createDebugCalllback(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack, void* userData)
{
    if (s_vkCreateDebugReportCallbackEXT == VK_NULL_HANDLE)
    {
        s_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(VulkanWrapper::GetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
        ASSERT(s_vkCreateDebugReportCallbackEXT, "Can't get address");
    }

    VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
    dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    dbgCreateInfo.pNext = nullptr;
    dbgCreateInfo.flags = flags;
    dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)VulkanDebug::messageCallback;
    dbgCreateInfo.pUserData = userData;

    VkDebugReportCallbackEXT& callback = (callBack != VK_NULL_HANDLE) ? callBack : VulkanDebug::s_msgCallback;

    VkResult result = s_vkCreateDebugReportCallbackEXT(instance, &dbgCreateInfo, nullptr, &callback);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::createDebugCalllback: vkCreateDebugReportCallbackEXT error %s", vk::ErrorString(result).c_str());
    }
}

void VulkanDebug::freeDebugCallback(VkInstance instance)
{
    if (s_vkDestroyDebugReportCallbackEXT == VK_NULL_HANDLE)
    {
        s_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(VulkanWrapper::GetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        ASSERT(s_vkCreateDebugReportCallbackEXT, "Can't get address");
    }

    if (VulkanDebug::s_msgCallback != VK_NULL_HANDLE)
    {
        s_vkDestroyDebugReportCallbackEXT(instance, VulkanDebug::s_msgCallback, nullptr);
    }
}

bool VulkanDebug::checkLayerIsSupported(const c8* layerName)
{
    u32 instanceLayerPropertyCount = 0;
    VkResult result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("DebugVK::isValidationLayerSupported: vkEnumerateInstanceLayerProperties count error %s", vk::ErrorString(result).c_str());
        return false;
    }

    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    if (!instanceLayerProperties.empty())
    {
        result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("DebugVK::isValidationLayerSupported: vkEnumerateInstanceLayerProperties list error %s", vk::ErrorString(result).c_str());
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

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
