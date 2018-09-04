#include "VulkanSwapchain.h"
#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

#ifdef PLATFORM_WINDOWS
bool createSurfaceWinApi(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd, VkSurfaceKHR& surface)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = hInstance;
    surfaceCreateInfo.hwnd = hWnd;

    VkResult result = vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, VULKAN_ALLOCATOR, &surface);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("createSurfaceWinApi: vkCreateWin32SurfaceKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}
#endif //PLATFORM_WINDOWS

VulkanSwapchain::VulkanSwapchain(const DeviceInfo* info, VkSurfaceKHR surface)
    : m_deviceInfo(info)
    , m_surface(surface)
    , m_swapchain(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanSwapchain constructor %llx", this);
}

VulkanSwapchain::~VulkanSwapchain()
{
    LOG_DEBUG("VulkanSwapchain destructor %llx", this);

    ASSERT(m_swapchain == VK_NULL_HANDLE, "swapchain is not nullptr");
}

VkSurfaceKHR VulkanSwapchain::createSurface(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
#ifdef PLATFORM_WINDOWS
    if (!createSurfaceWinApi(vkInstance, hInstance, hWnd, surface))
    {
        LOG_FATAL("VulkanSwapchain::createSurface: Create win surface is falied");
        return VK_NULL_HANDLE;
    }
#else
    LOG_FATAL("VulkanSwapchain::createSurface: paltform not supported");
    return false;
#endif

    LOG_DEBUG("VulkanSwapchain::createSurface created %llu", surface);
    return surface;
}

void VulkanSwapchain::detroySurface(VkInstance vkInstance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(vkInstance, surface, VULKAN_ALLOCATOR);
}

bool VulkanSwapchain::create()
{
    LOG_DEBUG("VulkanSwapchain::create");

    VkSurfaceCapabilitiesKHR surfaceCaps;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_deviceInfo->_physicalDevice, m_surface, &surfaceCaps);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceCapabilitiesKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    if (surfaceCaps.maxImageCount < 2)
    {
        LOG_ERROR("VulkanSwapchain::create: Not enough images supported in vulkan swapchain");
        return false;
    }

    VkBool32 supportsPresentation = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_deviceInfo->_physicalDevice, m_deviceInfo->_queueFamilyIndex, m_surface, &supportsPresentation);
    if (!supportsPresentation)
    {
        LOG_ERROR("VulkanSwapchain::create: not support presentation");
        return false;
    }

    //Get Surface format
    u32 surfaceFormatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    std::for_each(surfaceFormats.begin(), surfaceFormats.end(), [](const VkSurfaceFormatKHR& surfaceFormat)
    {
        LOG_INFO("SurfaceFormat supported format: %d, colorspace: %d", surfaceFormat.format, surfaceFormat.colorSpace);
    });

    u32 surfaceFormatIndex = 0;
    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((surfaceFormatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) //???
    {
        m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        // Always select the first available color format
        // If you need a specific format (e.g. SRGB) you'd need to
        // iterate over the list of available surface format and
        // check for it's presence
        m_surfaceFormat.format = surfaceFormats[0].format;
    }
    m_surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;


    if (!VulkanSwapchain::createSwapchain())
    {
        LOG_FATAL("VulkanSwapchain::createSwapchain Can not create swapchain");
        return false;
    }

    return true;
}

bool VulkanSwapchain::createSwapchain()
{
    //TODO:
    return false;
}

void VulkanSwapchain::destroy()
{
}

void VulkanSwapchain::present()
{
}

void VulkanSwapchain::acquireImage()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
