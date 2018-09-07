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

    VkResult result = VulkanWrapper::CreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, VULKAN_ALLOCATOR, &surface);
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
    VulkanWrapper::DestroySurfaceKHR(vkInstance, surface, VULKAN_ALLOCATOR);
}

bool VulkanSwapchain::create(const SwapchainConfig& config)
{
    LOG_DEBUG("VulkanSwapchain::create");

    VkResult result = VulkanWrapper::GetPhysicalDeviceSurfaceCapabilitiesKHR(m_deviceInfo->_physicalDevice, m_surface, &m_surfaceCaps);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceCapabilitiesKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    if (m_surfaceCaps.maxImageCount < 2)
    {
        LOG_ERROR("VulkanSwapchain::create: Not enough images supported in vulkan swapchain");
        return false;
    }

    VkBool32 supportsPresentation = false;
    VulkanWrapper::GetPhysicalDeviceSurfaceSupportKHR(m_deviceInfo->_physicalDevice, m_deviceInfo->_queueFamilyIndex, m_surface, &supportsPresentation);
    if (!supportsPresentation)
    {
        LOG_ERROR("VulkanSwapchain::create: not support presentation");
        return false;
    }

    //Get Surface format
    u32 surfaceFormatCount;
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());
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


    if (!VulkanSwapchain::createSwapchain(config))
    {
        LOG_FATAL("VulkanSwapchain::createSwapchain Can not create swapchain");
        return false;
    }

    if (!VulkanSwapchain::createSwapchainImages())
    {
        LOG_FATAL(" VulkanSwapchain::createSwapchainImages: cannot create swapchain images");
        return false;
    }

    return true;
}

bool VulkanSwapchain::createSwapchain(const SwapchainConfig& config)
{
    ASSERT(m_surface, "surface is nullptr");

    // Select a present mode for the swapchain
    u32 presentModeCount = 0;
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModesKHR(m_deviceInfo->_physicalDevice, m_surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModesKHR(m_deviceInfo->_physicalDevice, m_surface, &presentModeCount, presentModes.data());

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!config._vsync)
    {
        for (u32 i = 0; i < presentModeCount; i++)
        {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    // Determine the number of images
    uint32_t desiredNumberOfSwapchainImages = m_surfaceCaps.minImageCount + 1;
    if ((m_surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > m_surfaceCaps.maxImageCount))
    {
        desiredNumberOfSwapchainImages = m_surfaceCaps.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR preTransform;
    if (m_surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = m_surfaceCaps.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.pNext = nullptr;
    swapChainInfo.surface = m_surface;
    swapChainInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapChainInfo.imageFormat = m_surfaceFormat.format;
    swapChainInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = { config._size.width, config._size.height };
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapChainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
    swapChainInfo.presentMode = swapchainPresentMode;
    swapChainInfo.oldSwapchain = nullptr;
    swapChainInfo.clipped = VK_TRUE; // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapChainInfo.compositeAlpha = (m_surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;;

    VkResult result = VulkanWrapper::CreateSwapchainKHR(m_deviceInfo->_device, &swapChainInfo, VULKAN_ALLOCATOR, &m_swapchain);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapChain: vkCreateSwapchainKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_DEBUG("SwapChainVK::createSwapChain created");

    return true;
}

bool VulkanSwapchain::createSwapchainImages()
{
    u32 swapChainImageCount;
    VkResult result = vkGetSwapchainImagesKHR(m_deviceInfo->_device, m_swapchain, &swapChainImageCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapchainImages: vkGetSwapchainImagesKHR count. Error %s", ErrorString(result).c_str());
        return false;
    }

    if (swapChainImageCount < 2)
    {
        LOG_ERROR("VulkanSwapchain::createSurface: Not enough images supported in vulkan swapchain");
        return false;
    }

    LOG_DEBUG("SwapChainVK::createSwapchainImages: Count images %d", swapChainImageCount);

    std::vector<VkImage> images(swapChainImageCount);
    result = vkGetSwapchainImagesKHR(m_deviceInfo->_device, m_swapchain, &swapChainImageCount, images.data());
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapchainImages: vkGetSwapchainImagesKHR array. Error %s", ErrorString(result).c_str());
        return false;
    }

    m_swapBuffers.reserve(swapChainImageCount);
    for (auto& image : images)
    {
        /*TextureVK* texture = new TextureVK(ETextureTarget::eTextureRectangle, EImageFormat::eRGBA, EImageType::eUnsignedByte, core::Dimension3D(m_surfaceSize.width, m_surfaceSize.height, 1), nullptr);
        if (!texture->create(image))
        {
            LOG_FATAL("VulkanSwapchain::createSwapchainImages: can't create surface texture");
            delete texture;
        }
        m_swapBuffers.push_back(texture);*/
    }

    return true;
}

void VulkanSwapchain::destroy()
{
    VulkanWrapper::DestroySwapchainKHR(m_deviceInfo->_device, m_swapchain, VULKAN_ALLOCATOR);
    m_swapchain = VK_NULL_HANDLE;
}

void VulkanSwapchain::present()
{
    //TODO:
}

void VulkanSwapchain::acquireImage()
{
    //TODO:
}

bool VulkanSwapchain::recteateSwapchain(const SwapchainConfig& config)
{
    VulkanSwapchain::destroy();
    //TODO: remove images
    if (!VulkanSwapchain::createSwapchain(config))
    {
        LOG_FATAL("VulkanSwapchain::recteateSwapchain: is failed");
        return false;
    }
    //TODO: create images

    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
