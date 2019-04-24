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

#ifdef VK_KHR_create_renderpass2
#define ENUM_VK_CREATE_RENDERPASS2_FUNCTIONS(Macro) \
    Macro(vkCreateRenderPass2KHR) \
    Macro(vkCmdBeginRenderPass2KHR) \
    Macro(vkCmdNextSubpass2KHR) \
    Macro(vkCmdEndRenderPass2KHR)
#endif // VK_KHR_create_renderpass2

#define ENUM_VK_ALL_FUNCTIONS(Macro) \
        ENUM_VK_DEBUG_UTILS_FUNCTIONS(Macro) \
        ENUM_VK_CREATE_RENDERPASS2_FUNCTIONS(Macro)

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

} //namespace vk
} //namespace renderer
} //namespace v3d

#undef DECLARE_VK_FUNCTIONS

#endif //VULKAN_RENDER
