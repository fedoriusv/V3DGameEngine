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

    std::string ErrorString(VkResult errorCode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDebug
    {
    public:

        static const u16    s_severityDebugLevel = 4;

        static bool         createDebugUtilsMesseger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severityFlag, VkDebugUtilsMessageTypeFlagsEXT flags, PFN_vkDebugUtilsMessengerCallbackEXT callback, void* userData);
        static void         destroyDebugUtilsMesseger(VkInstance instance);

        static VkBool32 VKAPI_PTR defaultDebugUtilsMessegerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


        static bool         checkInstanceLayerIsSupported(const c8* layerName);
        static bool         checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName);

        static const std::vector<const c8*> s_validationLayerNames;

    private:

        static VkDebugUtilsMessengerEXT s_messeger;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
