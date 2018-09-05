#pragma once
#include "Common.h"

#ifdef VULKAN_RENDER

#   ifdef PLATFORM_WINDOWS
#       define VK_USE_PLATFORM_WIN32_KHR
#   elif PLATFORM_LINUX
#       define VK_USE_PLATFORM_XLIB_KHR
#   elif PLATFORM_ANDROID
#       define VK_USE_PLATFORM_ANDROID_KHR
#   endif

//#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>


#ifdef VK_EXT_debug_utils
#define ENUM_VK_DEBUG_UTILS_FUNCTIONS(Macro) \
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
#endif //VK_EXT_debug_utils

#ifdef VULKAN_DYNAMIC
#define DECLARE_VK_FUNCTIONS(Func) extern PFN_##Func Func;
#else
#define DECLARE_VK_FUNCTIONS(Func) static PFN_##Func Func = nullptr;
#endif //VULKAN_DYNAMIC

namespace v3d
{
namespace renderer
{
namespace vk
{
    extern PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT1;
    //ENUM_VK_DEBUG_UTILS_FUNCTIONS(DECLARE_VK_FUNCTIONS);


    bool LoadVulkanLibrary();
    bool LoadVulkanLibrary(VkInstance instance);
    bool LoadVulkanLibrary(VkDevice device);

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
