#include "VulkanFunctions.h"

#ifdef VULKAN_RENDER
#   ifdef PLATFORM_WINDOWS
#       pragma comment(lib, "vulkan-1.lib")
#   endif //PLATFORM_WINDOWS

namespace v3d
{
namespace renderer
{
namespace vk
{

PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT1 = nullptr;

bool LoadVulkanLibrary()
{
    return false;
}

bool LoadVulkanLibrary(VkInstance instance)
{
#ifdef VULKAN_DYNAMIC
#   error "Not implemented"
    return false;
#else

    ASSERT(&vkGetInstanceProcAddr, "vkGetInstanceProcAddr has not addres");

    vkCreateDebugUtilsMessengerEXT1 = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    //vkCreateDebugUtilsMessengerEXT = vkCreateDebugUtilsMessenger;
//#   define GET_ADDRES_VK_FUNCTIONS(Func) Func = reinterpret_cast<PFN_##Func>(vkGetInstanceProcAddr(instance, #Func));
//    ENUM_VK_DEBUG_UTILS_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
//
//    if (&vkCreateDebugUtilsMessengerEXT)
//    {
//        int a = 0;
//    }
//
//#   undef GET_ADDRES_VK_FUNCTIONS

    return true;
#endif //VULKAN_DYNAMIC
}

bool LoadVulkanLibrary(VkDevice device)
{
    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
