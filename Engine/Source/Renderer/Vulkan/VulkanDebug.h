#pragma once

#include "Common.h"
#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if USE_VULKAN_ALLOCATOR
#   define VULKAN_ALLOCATOR nullptr
#else
#   define VULKAN_ALLOCATOR nullptr
#endif //USE_VULKAN_ALLOCATOR

    std::string  ErrorString(VkResult errorCode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDebug
    {
    public:

        static VkBool32                 messageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, u64 srcObject, size_t location, s32 msgCode, c8* layerPrefix, c8* msg, void* userData);

        static void                     createDebugCalllback(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack, void* userData);
        static void                     freeDebugCallback(VkInstance instance);

        static bool                     checkInstanceLayerIsSupported(const c8* layerName);
        static bool                     checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName);

        static VkDebugReportCallbackEXT s_msgCallback;

        static const bool               s_enableValidationLayers;
        static const bool               s_enableDebugMarkers;

        static const std::vector<const c8*> s_validationLayerNames;

    private:

        static PFN_vkCreateDebugReportCallbackEXT   s_vkCreateDebugReportCallbackEXT;
        static PFN_vkDestroyDebugReportCallbackEXT  s_vkDestroyDebugReportCallbackEXT;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
