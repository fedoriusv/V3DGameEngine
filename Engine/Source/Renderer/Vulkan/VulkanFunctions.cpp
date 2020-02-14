#include "VulkanFunctions.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER

#ifdef VULKAN_DYNAMIC
#   if defined(PLATFORM_ANDROID)
#       include <dlfcn.h>
void* g_libDynamicVulkan = nullptr;
#   endif //PLATFORM_ANDROID
#else //VULKAN_DYNAMIC
#   if defined(PLATFORM_WINDOWS)
#       pragma comment(lib, "vulkan-1.lib")
#   endif //PLATFORM_WINDOWS
#endif //VULKAN_DYNAMIC

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
#ifdef VULKAN_DYNAMIC
#   if defined(PLATFORM_ANDROID)
    LOG_INFO("LoadVulkanLibrary Loading libvulkan.so");
    g_libDynamicVulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!g_libDynamicVulkan)
    {
        LOG_FATAL("LoadVulkanLibrary Could not load vulkan library : %s", dlerror());
        return false;
    }

#   define LOAD_VK_ENTRYPOINTS(Func) Func = (PFN_##Func)dlsym(g_libDynamicVulkan, #Func);
    ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(LOAD_VK_ENTRYPOINTS);
#   undef LOAD_VK_ENTRYPOINTS

    bool validation = true;
#   if VULKAN_DEBUG
#       define CHECK_VK_FUNKTIONS(Func) if (!Func) { LOG_WARNING("LoadVulkanLibrary() funtion can't get address %s", #Func); validation = true; }
    ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(CHECK_VK_FUNKTIONS);
#       undef CHECK_VK_FUNKTIONS
#   endif //VULKAN_DEBUG
    return validation;
#   else //PLATFORM_ANDROID
#       error "Dynamic vulkan is not implemented"
    return false;
#   endif //PLATFORM_ANDROID

#else //VULKAN_DYNAMIC
    bool validation = true;
#   if VULKAN_DEBUG
#       define CHECK_VK_FUNKTIONS(Func) if (!Func) { LOG_WARNING("LoadVulkanLibrary() funtion can't get address %s", #Func); validation = true; }
    ENUM_VK_ENTRYPOINTS_BASE_FUNCTIONS(CHECK_VK_FUNKTIONS);
#       undef CHECK_VK_FUNKTIONS
#   endif //VULKAN_DEBUG
    return validation;

#endif //VULKAN_DYNAMIC
}

bool LoadVulkanLibrary(VkInstance instance)
{
    ASSERT(&vkGetInstanceProcAddr, "vkGetInstanceProcAddr has not address");

#   define GET_ADDRES_VK_FUNCTIONS(Func) Func = reinterpret_cast<PFN_##Func>(vkGetInstanceProcAddr(instance, #Func));
    ENUM_VK_ENTRYPOINTS_CORE_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_ENTRYPOINTS_CORE_1_1_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_ENTRYPOINTS_SURFACE_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_ENTRYPOINTS_SWAPCHAIN_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_CREATE_RENDERPASS2_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_DEBUG_REPORT_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
    ENUM_VK_DEBUG_UTILS_FUNCTIONS(GET_ADDRES_VK_FUNCTIONS);
#   undef GET_ADDRES_VK_FUNCTIONS

    bool validation = true;
#   if VULKAN_DEBUG
#       define CHECK_VK_FUNKTIONS(Func) if (!Func) { LOG_WARNING("LoadVulkanLibrary(VkInstance instance) funtion can't get address %s", #Func); validation = true; }
    ENUM_VK_ENTRYPOINTS_CORE_FUNCTIONS(CHECK_VK_FUNKTIONS);
    ENUM_VK_ENTRYPOINTS_SURFACE_FUNCTIONS(CHECK_VK_FUNKTIONS);
    ENUM_VK_ENTRYPOINTS_SWAPCHAIN_FUNCTIONS(CHECK_VK_FUNKTIONS);
    ENUM_VK_ENTRYPOINTS_PLATFORM_FUNCTIONS(CHECK_VK_FUNKTIONS);
#       undef CHECK_VK_FUNKTIONS
#   endif //VULKAN_DEBUG
    return validation;
}

bool LoadVulkanLibrary(VkDevice device)
{
#   define GET_ADDRES_VK_FUNCTIONS(Func) Func = reinterpret_cast<PFN_##Func>(vkGetDeviceProcAddr(device, #Func));
//TODO
#   undef GET_ADDRES_VK_FUNCTIONS

    return true;
}

void FreeVulkanLibrary()
{
#ifdef VULKAN_DYNAMIC
#   if defined(PLATFORM_ANDROID)
    if (g_libDynamicVulkan)
    {
        LOG_INFO("FreeVulkanLibrary free libvulkan.so");

#   define CLEAR_VK_ENTRYPOINTS(Func) Func = nullptr;
        ENUM_VK_ALL_FUNCTIONS(CLEAR_VK_ENTRYPOINTS);
#   undef CLEAR_VK_ENTRYPOINTS

        dlclose(g_libDynamicVulkan);
        g_libDynamicVulkan = nullptr;
    }
#   endif //PLATFORM_ANDROID
#endif //VULKAN_DYNAMIC
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
