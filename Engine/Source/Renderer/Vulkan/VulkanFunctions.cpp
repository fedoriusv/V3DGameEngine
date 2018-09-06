#include "VulkanFunctions.h"
#include "Utils/Logger.h"

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

#define INITIALIZE_VK_FUNCTIONS(Func) PFN_##Func Func = nullptr;
    ENUM_VK_ALL_FUNCTIONS(INITIALIZE_VK_FUNCTIONS);
#undef INITIALIZE_VK_FUNCTIONS

bool LoadVulkanLibrary()
{
    //TODO:
    return true;
}

bool LoadVulkanLibrary(VkInstance instance)
{
#ifdef VULKAN_DYNAMIC
#   error "Dynamic vulkan is not implemented"
    return false;
#else
    ASSERT(&vkGetInstanceProcAddr, "vkGetInstanceProcAddr has not address");

#   define GET_ADDRES_VK_FUNCTIONS(Func) Func = reinterpret_cast<PFN_##Func>(vkGetInstanceProcAddr(instance, #Func));
    ENUM_VK_ALL_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
#   undef GET_ADDRES_VK_FUNCTIONS

    bool validation = true;
#if VULKAN_DEBUG
#   define CHECK_VK_FUNKTIONS(Func) if (!Func) { LOG_WARNING("LoadVulkanLibrary(VkInstance instance) funtion can't get address %s", #Func); validation = true; }
    ENUM_VK_ALL_FUNCTIONS(CHECK_VK_FUNKTIONS);
#   undef CHECK_VK_FUNKTIONS
#endif //VULKAN_DEBUG
    return validation;
#endif //VULKAN_DYNAMIC
}

bool LoadVulkanLibrary(VkDevice device)
{
    //TODO:
    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
