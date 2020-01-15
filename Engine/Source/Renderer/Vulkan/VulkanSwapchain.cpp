#include "VulkanSwapchain.h"
#include "VulkanGraphicContext.h"
#include "VulkanImage.h"
#include "VulkanResource.h"
#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_ANDROID
#   include "Platform/Android/AndroidCommon.h"
#   include "Platform/Android/android_native_app_glue.h"

extern android_app* g_nativeAndroidApp;
#endif //PLATFORM_ANDROID

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

    VkResult result = VulkanWrapper::CreateWin32Surface(vkInstance, &surfaceCreateInfo, VULKAN_ALLOCATOR, &surface);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("createSurfaceWinApi: vkCreateWin32SurfaceKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}
#endif //PLATFORM_WINDOWS

#ifdef PLATFORM_ANDROID
bool createSurfaceAndroidApi(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd, VkSurfaceKHR& surface)
{
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.window = hWnd;

    LOG_DEBUG("createSurfaceAndroidApi window %x", surfaceCreateInfo.window);

    VkResult result = VulkanWrapper::CreateAndroidSurface(vkInstance, &surfaceCreateInfo, VULKAN_ALLOCATOR, &surface);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("createSurfaceAndroidApi: vkCreateAndroidSurfaceKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}
#endif //PLATFORM_ANDROID

VulkanSwapchain::VulkanSwapchain(const DeviceInfo* info)
    : m_deviceInfo(info)
    , m_surface(VK_NULL_HANDLE)
    , m_surfaceCaps({})
    , m_surfaceFormat({})
    , m_swapchain(VK_NULL_HANDLE)

    , m_currentImageIndex((u32)-1)
    , m_currentSemaphoreIndex(0U)

    , m_ready(false)
{
    LOG_DEBUG("VulkanSwapchain constructor %llx", this);
}

VulkanSwapchain::~VulkanSwapchain()
{
    LOG_DEBUG("VulkanSwapchain destructor %llx", this);

    ASSERT(!m_ready, "doesnt deleted");

    ASSERT(m_swapBuffers.empty(), "not empty");
    ASSERT(!m_swapchain, "swapchain is not nullptr");
    ASSERT(m_acquireSemaphore.empty(), "not empty");
    ASSERT(!m_surface, "surface isn't nullptr");
}

VkSurfaceKHR VulkanSwapchain::createSurface(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(PLATFORM_WINDOWS)
    if (!createSurfaceWinApi(vkInstance, hInstance, hWnd, surface))
    {
        LOG_FATAL("VulkanSwapchain::createSurface: Create win surface is falied");
        return VK_NULL_HANDLE;
    }
#elif defined (PLATFORM_ANDROID)
    if (!createSurfaceAndroidApi(vkInstance, hInstance, hWnd, surface))
    {
        LOG_FATAL("VulkanSwapchain::createSurface: Create android surface is falied");
        return VK_NULL_HANDLE;
    }
#else
    LOG_FATAL("VulkanSwapchain::createSurface: paltform not supported");
    return false;
#endif

    LOG_DEBUG("VulkanSwapchain::createSurface created %llx", surface);
    return surface;
}

bool VulkanSwapchain::create(const SwapchainConfig& config)
{
    LOG_DEBUG("VulkanSwapchain::create");
    if (m_ready)
    {
        return true;
    }

    m_config = config;

    ASSERT(!m_surface, "Already has created");
    m_surface = VulkanSwapchain::createSurface(m_deviceInfo->_instance, config._window->getInstance(), config._window->getWindowHandle());
    if (!m_surface)
    {
        VulkanSwapchain::destroy();
        LOG_FATAL("VulkanGraphicContext::createContext: Can not create VkSurfaceKHR");
        return false;
    }

    VkResult result = VulkanWrapper::GetPhysicalDeviceSurfaceCapabilities(m_deviceInfo->_physicalDevice, m_surface, &m_surfaceCaps);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceCapabilitiesKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    if ((config._size.width < m_surfaceCaps.minImageExtent.width || config._size.width > m_surfaceCaps.maxImageExtent.width) ||
        (config._size.height < m_surfaceCaps.minImageExtent.height || config._size.height > m_surfaceCaps.maxImageExtent.height))
    {
        LOG_ERROR("VulkanSwapchain::create: Not support swapchain size min[%d, %d], max[%d, %d], requested [%d, %d]", 
            m_surfaceCaps.minImageExtent.width, m_surfaceCaps.minImageExtent.height, m_surfaceCaps.maxImageExtent.width, m_surfaceCaps.maxImageExtent.height, config._size.width, config._size.height);
        return false;
    }

    if (m_surfaceCaps.maxImageCount < 2)
    {
        LOG_ERROR("VulkanSwapchain::create: Not enough images supported in vulkan swapchain");
        return false;
    }

    VkBool32 supportsPresentation = false;
    VulkanWrapper::GetPhysicalDeviceSurfaceSupport(m_deviceInfo->_physicalDevice, m_deviceInfo->_queueFamilyIndex, m_surface, &supportsPresentation);
    if (!supportsPresentation)
    {
        LOG_ERROR("VulkanSwapchain::create: not support presentation");
        return false;
    }

    //Get Surface format
    u32 surfaceFormatCount;
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormats(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormats(m_deviceInfo->_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    std::for_each(surfaceFormats.begin(), surfaceFormats.end(), [](const VkSurfaceFormatKHR& surfaceFormat)
    {
        LOG_INFO("SurfaceFormat supported format: %d, colorspace: %d", surfaceFormat.format, surfaceFormat.colorSpace);
    });

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((surfaceFormatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) //???
    {
        m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        m_surfaceFormat.format = surfaceFormats[0].format;
    }
    m_surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;


    if (!VulkanSwapchain::createSwapchain(config))
    {
         VulkanSwapchain::destroy();

        LOG_FATAL("VulkanSwapchain::createSwapchain Can not create swapchain");
        return false;
    }

    if (!VulkanSwapchain::createSwapchainImages(config))
    {
        VulkanSwapchain::destroy();

        LOG_FATAL(" VulkanSwapchain::createSwapchainImages: cannot create swapchain images");
        return false;
    }

    m_acquireSemaphore.reserve(m_swapBuffers.size());
    for (u32 index = 0; index < m_swapBuffers.size(); ++index)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkResult result = VulkanWrapper::CreateSemaphore(m_deviceInfo->_device, &semaphoreCreateInfo, VULKAN_ALLOCATOR, &semaphore);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR(" VulkanSwapchain::create Acquire Semaphore vkCreateSemaphore is failed. Error %s", ErrorString(result).c_str());
        }
        m_acquireSemaphore.push_back(semaphore);
    }

    m_ready = true;
    return true;
}

bool VulkanSwapchain::createSwapchain(const SwapchainConfig& config)
{
    ASSERT(m_surface, "surface is nullptr");

    // Select a present mode for the swapchain
    u32 presentModeCount = 0;
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModes(m_deviceInfo->_physicalDevice, m_surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModes(m_deviceInfo->_physicalDevice, m_surface, &presentModeCount, presentModes.data());

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
    uint32_t desiredNumberOfSwapchainImages = m_surfaceCaps.minImageCount;
    if (config._countSwapchainImages == 0)
    {
        desiredNumberOfSwapchainImages = m_surfaceCaps.minImageCount + 1;
        if ((m_surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > m_surfaceCaps.maxImageCount))
        {
            desiredNumberOfSwapchainImages = m_surfaceCaps.maxImageCount;
        }
    }
    else
    {
        ASSERT(m_surfaceCaps.minImageCount <= config._countSwapchainImages && config._countSwapchainImages <= m_surfaceCaps.maxImageCount, "range out");
        desiredNumberOfSwapchainImages = std::clamp(config._countSwapchainImages, m_surfaceCaps.minImageCount, m_surfaceCaps.maxImageCount);
    }
    LOG_DEBUG("SwapChainVK::createSwapChain swapchain images count(min %u, max: %u), chosen: %u", m_surfaceCaps.minImageCount, m_surfaceCaps.maxImageCount, desiredNumberOfSwapchainImages);

    // Find the transformation of the surface
    VkSurfaceTransformFlagBitsKHR preTransform;
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
    swapChainInfo.flags = 0;
    swapChainInfo.surface = m_surface;
    swapChainInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapChainInfo.imageFormat = m_surfaceFormat.format;
    swapChainInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = { config._size.width, config._size.height };
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapChainInfo.preTransform = preTransform;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
    swapChainInfo.presentMode = swapchainPresentMode;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainInfo.clipped = VK_TRUE; // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapChainInfo.compositeAlpha = (m_surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;;

    VkResult result = VulkanWrapper::CreateSwapchain(m_deviceInfo->_device, &swapChainInfo, VULKAN_ALLOCATOR, &m_swapchain);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapChain: vkCreateSwapchainKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_DEBUG("SwapChainVK::createSwapChain has been created");

    return true;
}

bool VulkanSwapchain::createSwapchainImages(const SwapchainConfig& config)
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
        LOG_ERROR("VulkanSwapchain::createSwapchainImages: Not enough images supported in vulkan swapchain");
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

    if (m_swapBuffers.empty())
    {
        m_swapBuffers.reserve(swapChainImageCount);
        for (auto& image : images)
        {
            VkExtent3D extent = { config._size.width, config._size.height, 1 };
            VulkanImage* swapchainImage = new VulkanImage(nullptr, m_deviceInfo->_device, m_surfaceFormat.format, extent, VK_SAMPLE_COUNT_1_BIT, 
                TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Read);
            if (!swapchainImage->create(image))
            {
                LOG_FATAL("VulkanSwapchain::createSwapchainImages: can't create surface texture");

                swapchainImage->destroy();
                delete swapchainImage;
            }
            else
            {
                m_swapBuffers.push_back(swapchainImage);
            }
        }
    }
    else
    {
        ASSERT(m_swapBuffers.size() == swapChainImageCount, "different size");
        std::vector<VkImage>::const_iterator imageIter = images.cbegin();
        for (auto& swapchainImage : m_swapBuffers)
        {
            ASSERT(imageIter != images.cend(), "wrong iterrator");
            if (!swapchainImage->create(*imageIter))
            {
                LOG_FATAL("VulkanSwapchain::createSwapchainImages: can't recreate surface image");
                for (auto& deletedSwapchainImage : m_swapBuffers)
                {
                    deletedSwapchainImage->destroy();
                    delete deletedSwapchainImage;
                }
                m_swapBuffers.clear();

                return false;
            }
            imageIter = std::next(imageIter);
        }
    }

    return true;
}

void VulkanSwapchain::destroy()
{
    if (!m_swapBuffers.empty())
    {
        for (std::vector<VulkanImage*>::iterator imageIter = m_swapBuffers.begin(); imageIter < m_swapBuffers.end(); ++imageIter)
        {
            VulkanImage* image = *imageIter;
            if (image)
            {
                image->destroy();

                delete image;
                image = nullptr;
            }
        }
    }
    m_swapBuffers.clear();

    if (m_swapchain)
    {
        VulkanWrapper::DestroySwapchain(m_deviceInfo->_device, m_swapchain, VULKAN_ALLOCATOR);
        m_swapchain = VK_NULL_HANDLE;
    }

    for (auto& semaphore : m_acquireSemaphore)
    {
        VulkanWrapper::DestroySemaphore(m_deviceInfo->_device, semaphore, VULKAN_ALLOCATOR);
    }
    m_acquireSemaphore.clear();

    if (m_surface)
    {
        VulkanWrapper::DestroySurface(m_deviceInfo->_instance, m_surface, VULKAN_ALLOCATOR);
        m_surface = VK_NULL_HANDLE;
    }

    m_swapchainResources.clear();

    m_ready = false;
}

void VulkanSwapchain::present(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores)
{
    ASSERT(m_swapchain, "m_swapchain is nullptr");

    VkResult innerResults[1] = {};

    VkPresentInfoKHR presentInfoKHR = {};
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = nullptr;
    if (waitSemaphores.empty())
    {
        presentInfoKHR.waitSemaphoreCount = 1;
        presentInfoKHR.pWaitSemaphores = &m_acquireSemaphore[m_currentSemaphoreIndex];
    }
    else
    {
        presentInfoKHR.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
        presentInfoKHR.pWaitSemaphores = waitSemaphores.data();
    }
    presentInfoKHR.swapchainCount = 1;
    presentInfoKHR.pSwapchains = &m_swapchain;
    presentInfoKHR.pImageIndices = &m_currentImageIndex;
    presentInfoKHR.pResults = innerResults;

    VkResult result = VulkanWrapper::QueuePresent(queue, &presentInfoKHR);
    if (result == VK_ERROR_SURFACE_LOST_KHR || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanWrapper::DeviceWaitIdle(m_deviceInfo->_device);
#ifdef PLATFORM_ANDROID
    ASSERT(g_nativeAndroidApp->window, "windows is nullptr");
#endif //ANDROID_PLATFORM
        //recreate
        LOG_WARNING("VulkanSwapchain::present: Swapchain need to recreate. Error: %s", ErrorString(result).c_str());
        if (!VulkanSwapchain::recteate(m_config))
        {
            LOG_FATAL(" VulkanSwapchain::QueuePresent: recteate was failed");
        }

        recreateAttachedResources();
    }
    else if (result != VK_SUCCESS)
    {
        LOG_FATAL(" VulkanSwapchain::QueuePresent: failed with error %s", ErrorString(result).c_str());
        ASSERT(false, "QueuePresent failed");
    }
    m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_acquireSemaphore.size();
}

u32 VulkanSwapchain::acquireImage()
{
    VkSemaphore semaphore = m_acquireSemaphore[m_currentSemaphoreIndex];
    VkFence fence = VK_NULL_HANDLE;

    u32 imageIndex = 0;
    VkResult result = VulkanWrapper::AcquireNextImage(m_deviceInfo->_device, m_swapchain, UINT64_MAX, semaphore, fence, &imageIndex);
    if (result == VK_ERROR_SURFACE_LOST_KHR || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanWrapper::DeviceWaitIdle(m_deviceInfo->_device);
#ifdef PLATFORM_ANDROID
    ASSERT(g_nativeAndroidApp->window, "windows is nullptr");
#endif //ANDROID_PLATFORM

        //recreate
        LOG_WARNING(" VulkanSwapchain::acquireImage: Swapchain need to recreate. Error: %s", ErrorString(result).c_str());
        if (!VulkanSwapchain::recteate(m_config))
        {
            LOG_FATAL(" VulkanSwapchain::AcquireNextImage: recteate was failed");
        }
        recreateAttachedResources();

        VkSemaphore semaphoreInner = m_acquireSemaphore[m_currentSemaphoreIndex];

        VkResult resultInner = VulkanWrapper::AcquireNextImage(m_deviceInfo->_device, m_swapchain, UINT64_MAX, semaphoreInner, VK_NULL_HANDLE, &imageIndex);
        if (resultInner != VK_SUCCESS)
        {
            LOG_FATAL(" VulkanSwapchain::AcquireNextImage: failed recreate with error %s", ErrorString(result).c_str());
            ASSERT(false, "vkAcquireNextImageKHR failed recreate");
        }
    }
    else if (result != VK_SUCCESS)
    {
        LOG_FATAL(" VulkanSwapchain::AcquireNextImage: failed with error %s", ErrorString(result).c_str());
        ASSERT(false, "vkAcquireNextImageKHR failed");
    }

    m_currentImageIndex = imageIndex;
    return imageIndex;
}

bool VulkanSwapchain::recteate(const SwapchainConfig& config)
{
    if (!m_ready)
    {
        return false;
    }

    for (std::vector<VulkanImage *>::iterator image = m_swapBuffers.begin(); image < m_swapBuffers.end(); ++image)
    {
        (*image)->destroy();
    }

    if (m_swapchain)
    {
        VulkanWrapper::DestroySwapchain(m_deviceInfo->_device, m_swapchain, VULKAN_ALLOCATOR);
        m_swapchain = VK_NULL_HANDLE;
    }

    for (auto& semaphore : m_acquireSemaphore)
    {
        VulkanWrapper::DestroySemaphore(m_deviceInfo->_device, semaphore, VULKAN_ALLOCATOR);
    }
    m_acquireSemaphore.clear();

    if (m_surface)
    {
        VulkanWrapper::DestroySurface(m_deviceInfo->_instance, m_surface, VULKAN_ALLOCATOR);
        m_surface = VK_NULL_HANDLE;
    }

    m_ready = false;


    if (!VulkanSwapchain::create(config))
    {
        LOG_FATAL("VulkanSwapchain::recteate: is failed");
        return false;
    }

    return true;
}

VulkanImage * VulkanSwapchain::getSwapchainImage(u32 index) const
{
    return m_swapBuffers[index];
}

VulkanImage * VulkanSwapchain::getBackbuffer() const
{
    ASSERT(m_currentImageIndex >= 0, "invalid index");
    return m_swapBuffers[m_currentImageIndex];
}

u32 VulkanSwapchain::getSwapchainImageCount() const
{
    return static_cast<u32>(m_swapBuffers.size());
}

void VulkanSwapchain::attachResource(VulkanResource* resource, const std::function<bool(VulkanResource*)>& recreator)
{
    m_swapchainResources.emplace_back(resource, recreator);
}

void VulkanSwapchain::recreateAttachedResources()
{
    for (auto& [resource, recreator] : m_swapchainResources)
    {
        recreator(resource);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
