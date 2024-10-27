#include "VulkanSwapchain.h"

#include "Platform/Window.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDevice.h"
#   include "VulkanImage.h"
#   include "VulkanSemaphore.h"

#define DEBUG_FENCE_ACQUIRE 0

#ifdef PLATFORM_ANDROID
#   include "Platform/Android/AndroidCommon.h"
#   include "Platform/Android/android_native_app_glue.h"

extern android_app* g_nativeAndroidApp;
#endif //PLATFORM_ANDROID


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
bool createSurfaceAndroidApi(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd, VkSurfaceKHR& surface, const core::Dimension2D& size)
{
    LOG_DEBUG("createSurfaceAndroidApi: createSurfaceAndroidApi: { %d, %d } ", size.width, size.height);

    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = hWnd;

    LOG_DEBUG("createSurfaceAndroidApi native window %llx", surfaceCreateInfo.window);

    VkResult result = VulkanWrapper::CreateAndroidSurface(vkInstance, &surfaceCreateInfo, VULKAN_ALLOCATOR, &surface);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("createSurfaceAndroidApi: vkCreateAndroidSurfaceKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}
#endif //PLATFORM_ANDROID

VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, VulkanSemaphoreManager* const semaphoreManager) noexcept
    : m_device(*device)

    , m_window(nullptr)
    , m_params({})
    , m_surface(VK_NULL_HANDLE)
    , m_surfaceCapabilities({})

    , m_swapchain(VK_NULL_HANDLE)
    , m_currentImageIndex(~0U)

    , m_semaphoreManager(semaphoreManager)
    , m_currentSemaphoreIndex(0U)

    , m_insideFrame(false)
    , m_ready(false)
{
    LOG_DEBUG("VulkanSwapchain constructor %llx", this);
}

VulkanSwapchain::~VulkanSwapchain()
{
    LOG_DEBUG("VulkanSwapchain destructor %llx", this);

    ASSERT(!m_ready, "doesn't deleted");

    ASSERT(m_swapchainImages.empty(), "not empty");
    ASSERT(!m_swapchain, "swapchain is not nullptr");
    ASSERT(m_acquiredSemaphores.empty(), "not empty");
    ASSERT(!m_surface, "surface isn't nullptr");
}

void VulkanSwapchain::beginFrame()
{
    ASSERT(!m_insideFrame, "must be outside");
    m_insideFrame = true;

    [[maybe_unused]] u32 prevImageIndex = VulkanSwapchain::currentSwapchainIndex();
    [[maybe_unused]] u32 index = acquireImage();

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::beginFrame %llu, image index %u", m_frameCounter, index);
#endif //VULKAN_DEBUG

#if SWAPCHAIN_ON_ADVANCE
    ASSERT(prevImageIndex != ~0U, "wrong index");
    m_currentTransitionState.transitionImages(drawBuffer, { { m_swapchain->getSwapchainImage(prevImageIndex), { 0, 1, 0, 1} } }, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
#endif //SWAPCHAIN_ON_ADVANCE

#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpFrameNumber(m_frameCounter);
#endif
}

void VulkanSwapchain::endFrame()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::endFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG

    ASSERT(m_insideFrame, "must be inside");
    m_insideFrame = false;
}

void VulkanSwapchain::presentFrame()
{
    ASSERT(!m_insideFrame, "must be outside");

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::presentFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG

    for (auto cmdList: m_cmdLists)
    {
        VulkanSwapchain::present(m_device.getQueueByMask(cmdList->getDeviceMask()), cmdList->getWaitSemaphores());
        cmdList->postPresent();
    }
    m_cmdLists.clear();

    ++m_frameCounter;
}

VkSurfaceKHR VulkanSwapchain::createSurface(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd, const math::Dimension2D& size)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(PLATFORM_WINDOWS)
    if (!createSurfaceWinApi(vkInstance, hInstance, hWnd, surface))
    {
        LOG_FATAL("VulkanSwapchain::createSurface: Create win surface is falied");
        return VK_NULL_HANDLE;
    }
#elif defined(PLATFORM_ANDROID)
    if (!createSurfaceAndroidApi(vkInstance, hInstance, hWnd, surface, size))
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

bool VulkanSwapchain::create(platform::Window* window, const SwapchainParams& params, VkSwapchainKHR oldSwapchain)
{
    LOG_DEBUG("VulkanSwapchain::create");
    ASSERT(window, "must be valid");
    if (m_ready)
    {
        return true;
    }

    math::Dimension2D size = params._size;
    ASSERT(window->getSize() == size, "must be same");

    ASSERT(!m_surface, "Already has created");
    m_surface = VulkanSwapchain::createSurface(m_device.getDeviceInfo()._instance, window->getInstance(), window->getWindowHandle(), size);
    if (!m_surface)
    {
        LOG_FATAL("VulkanSwapchain::createSurface: Can not create VkSurfaceKHR");
        return false;
    }

    if (m_device.getVulkanDeviceCaps()._preTransform || m_device.getVulkanDeviceCaps()._renderpassTransformQCOM)
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        VkResult result = VulkanWrapper::GetPhysicalDeviceSurfaceCapabilities(m_device.getDeviceInfo()._physicalDevice, m_surface, &surfaceCapabilities);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceCapabilitiesKHR. Error %s", ErrorString(result).c_str());
            return false;
        }

        if (surfaceCapabilities.currentTransform & (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR))
        {
            VulkanSwapchain::destroy();

            std::swap(size.m_width, size.m_height);
            ASSERT(m_surface == VK_NULL_HANDLE, "must be nullptr");
            m_surface = VulkanSwapchain::createSurface(m_device.getDeviceInfo()._instance, window->getInstance(), window->getWindowHandle(), size);
            if (!m_surface)
            {
                LOG_FATAL("VulkanSwapchain::createSurface: Can not create VkSurfaceKHR");
                return false;
            }
        }
    }

    VkResult result = VulkanWrapper::GetPhysicalDeviceSurfaceCapabilities(m_device.getDeviceInfo()._physicalDevice, m_surface, &m_surfaceCapabilities);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceCapabilitiesKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    if ((size.m_width < m_surfaceCapabilities.minImageExtent.width || size.m_width > m_surfaceCapabilities.maxImageExtent.width) ||
        (size.m_height < m_surfaceCapabilities.minImageExtent.height || size.m_height > m_surfaceCapabilities.maxImageExtent.height))
    {
        u32 newWidth = math::clamp(size.m_width, m_surfaceCapabilities.minImageExtent.width, m_surfaceCapabilities.maxImageExtent.width);
        u32 newHeight = math::clamp(size.m_height, m_surfaceCapabilities.minImageExtent.height, m_surfaceCapabilities.maxImageExtent.height);

        LOG_WARNING("VulkanSwapchain::create: Is not supported swapchain size. min[%u, %u], max[%u, %u], requested[%u, %u], chosen[%u, %u]", 
            m_surfaceCapabilities.minImageExtent.width, m_surfaceCapabilities.minImageExtent.height, m_surfaceCapabilities.maxImageExtent.width, m_surfaceCapabilities.maxImageExtent.height,
            size.m_width, size.m_height, newWidth, newHeight);

        size.m_width = newWidth;
        size.m_height = newHeight;
    }

    if (m_surfaceCapabilities.maxImageCount < 2)
    {
        LOG_ERROR("VulkanSwapchain::create: Not enough images supported in vulkan swapchain");
        return false;
    }

    VkBool32 supportsPresentation = false;
    VulkanWrapper::GetPhysicalDeviceSurfaceSupport(m_device.getDeviceInfo()._physicalDevice, m_device.getQueueFamilyIndexByMask(Device::DeviceMask::GraphicMask), m_surface, &supportsPresentation);
    if (!supportsPresentation)
    {
        LOG_ERROR("VulkanSwapchain::create: not support presentation");
        return false;
    }

    //Get Surface format
    u32 surfaceFormatCount;
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormats(m_device.getDeviceInfo()._physicalDevice, m_surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    result = VulkanWrapper::GetPhysicalDeviceSurfaceFormats(m_device.getDeviceInfo()._physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSwapchain::create: vkGetPhysicalDeviceSurfaceFormatsKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    std::for_each(surfaceFormats.begin(), surfaceFormats.end(), [](const VkSurfaceFormatKHR& surfaceFormat)
    {
        LOG_INFO("SurfaceFormat supported format: %s, colorspace: %d", VulkanImage::imageFormatStringVK(surfaceFormat.format).c_str(), surfaceFormat.colorSpace);
    });

    VkSurfaceFormatKHR surfaceFormat{};

    auto findRequestedSurfaceFormat = [&surfaceFormats](VkFormat requestedFormat, VkSurfaceFormatKHR& surfaceFormat) -> bool
        {
            for (auto& colorFormat : surfaceFormats)
            {
                if (requestedFormat == colorFormat.format)
                {
                    surfaceFormat = colorFormat;
                    return true;
                }
            }

            return false;
        };

    bool formatIsFound = false;
    VkFormat requestedFormat = VulkanImage::convertImageFormatToVkFormat(params._format);
    if (requestedFormat != VK_FORMAT_UNDEFINED)
    {
        formatIsFound = findRequestedSurfaceFormat(requestedFormat, surfaceFormat);
    }

    if (!formatIsFound) //auto detect
    {
        //take first format from the list
        surfaceFormat.format = surfaceFormats[0].format;
        surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
    }

    if (!VulkanSwapchain::createSwapchain(params, surfaceFormat, oldSwapchain))
    {
        LOG_FATAL("VulkanSwapchain::createSwapchain Can not create swapchain");
        return false;
    }

    TextureUsageFlags flags = TextureUsage::TextureUsage_Backbuffer | TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Read;
    if (!VulkanSwapchain::createSwapchainImages(params, surfaceFormat, flags))
    {
        LOG_FATAL(" VulkanSwapchain::createSwapchainImages: cannot create swapchain images");
        return false;
    }

    u32 semaphoreCount = static_cast<u32>(m_swapchainImages.size());
    m_acquiredSemaphores.resize(semaphoreCount, nullptr);
    for (u32 index = 0; index < semaphoreCount; ++index)
    {
        std::string semaphoreName("AcquireSemaphore_" + std::to_string(index));
        m_acquiredSemaphores[index] = m_semaphoreManager->createSemaphore(semaphoreName);
    }

#if SWAPCHAIN_ON_ADVANCE
    u32 initialImageIndex = VulkanSwapchain::acquireImage();
    m_presentInfo = { initialImageIndex, m_acquireSemaphore[m_currentSemaphoreIndex] };
    m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % static_cast<u32>(m_acquireSemaphore.size());
#endif

    m_window = window;
    m_params = params;
    m_params._size = size;
    m_params._countSwapchainImages = static_cast<u32>(m_swapchainImages.size());
    m_params._format = VulkanImage::convertVkImageFormatToFormat(surfaceFormat.format);

    Swapchain::create(&m_device, VulkanImage::convertVkImageFormatToFormat(surfaceFormat.format), size, flags);

    m_ready = true;
    return true;
}

bool VulkanSwapchain::createSwapchain(const SwapchainParams& params, const VkSurfaceFormatKHR& surfaceFormat, VkSwapchainKHR oldSwapchain)
{
    LOG_DEBUG("VulkanSwapchain::createSwapchain size { %d, %d }", params._size.m_width, params._size.m_height);
    ASSERT(m_surface, "surface is nullptr");

    // Select a present mode for the swapchain
    u32 presentModeCount = 0;
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModes(m_device.getDeviceInfo()._physicalDevice, m_surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VulkanWrapper::GetPhysicalDeviceSurfacePresentModes(m_device.getDeviceInfo()._physicalDevice, m_surface, &presentModeCount, presentModes.data());

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!params._vSync)
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
    uint32_t desiredNumberOfSwapchainImages = m_surfaceCapabilities.minImageCount;
    if (params._countSwapchainImages == 0)
    {
        desiredNumberOfSwapchainImages = m_surfaceCapabilities.minImageCount + 1;
        if ((m_surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > m_surfaceCapabilities.maxImageCount))
        {
            desiredNumberOfSwapchainImages = m_surfaceCapabilities.maxImageCount;
        }
    }
    else
    {
        ASSERT(m_surfaceCapabilities.minImageCount <= params._countSwapchainImages && params._countSwapchainImages <= m_surfaceCapabilities.maxImageCount, "range out");
        desiredNumberOfSwapchainImages = std::clamp(params._countSwapchainImages, m_surfaceCapabilities.minImageCount, m_surfaceCapabilities.maxImageCount);
    }
    LOG_DEBUG("VulkanSwapchain::createSwapChain swapchain images count(min %u, max: %u), chosen: %u", m_surfaceCapabilities.minImageCount, m_surfaceCapabilities.maxImageCount, desiredNumberOfSwapchainImages);

    // Find the transformation of the surface
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if (m_device.getVulkanDeviceCaps()._preTransform || m_device.getVulkanDeviceCaps()._renderpassTransformQCOM)
    {
        preTransform = m_surfaceCapabilities.currentTransform;
    }
    else
    {
        if (m_surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            preTransform = m_surfaceCapabilities.currentTransform;
        }
    }

    const VkExtent2D imageExtent = { params._size.m_width, params._size.m_height };
    LOG_DEBUG("VulkanSwapchain::createSwapChain (width %u, height %u), currentTransform: %u, supportedTransforms: %u, selectedTransform: %u", 
        imageExtent.width, imageExtent.height, m_surfaceCapabilities.currentTransform, m_surfaceCapabilities.supportedTransforms, preTransform);

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.pNext = nullptr;
    swapChainInfo.flags = 0;
    swapChainInfo.surface = m_surface;
    swapChainInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = imageExtent;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapChainInfo.preTransform = preTransform;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
    swapChainInfo.presentMode = swapchainPresentMode;
    swapChainInfo.oldSwapchain = oldSwapchain;
    swapChainInfo.clipped = VK_TRUE; // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapChainInfo.compositeAlpha = (m_surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;;

    VkResult result = VulkanWrapper::CreateSwapchain(m_device.getDeviceInfo()._device, &swapChainInfo, VULKAN_ALLOCATOR, &m_swapchain);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapChain: vkCreateSwapchainKHR. Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_DEBUG("VulkanSwapchain::createSwapChain has been created");

    return true;
}

bool VulkanSwapchain::createSwapchainImages(const SwapchainParams& params, const VkSurfaceFormatKHR& surfaceFormat, TextureUsageFlags flags)
{
    u32 swapChainImageCount;
    VkResult result = vkGetSwapchainImagesKHR(m_device.getDeviceInfo()._device, m_swapchain, &swapChainImageCount, nullptr);
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

    LOG_DEBUG("VulkanSwapchain::createSwapchainImages: Count images %d", swapChainImageCount);

    std::vector<VkImage> images(swapChainImageCount);
    result = vkGetSwapchainImagesKHR(m_device.getDeviceInfo()._device, m_swapchain, &swapChainImageCount, images.data());
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanSwapchain::createSwapchainImages: vkGetSwapchainImagesKHR array. Error %s", ErrorString(result).c_str());
        return false;
    }

    if (m_swapchainImages.empty())
    {
        m_swapchainImages.reserve(swapChainImageCount);
        VkExtent3D extent = { params._size.m_width, params._size.m_height, 1 };
        for (u32 index = 0; index < images.size(); ++index)
        {
            VulkanImage* swapchainImage = V3D_NEW(VulkanImage, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_device.m_imageMemoryManager, surfaceFormat.format, extent, VK_SAMPLE_COUNT_1_BIT, 1U, flags, "SwapchainImage_" + std::to_string(index));
            if (!swapchainImage->create(images[index]))
            {
                LOG_FATAL("VulkanSwapchain::createSwapchainImages: can't create surface texture");

                swapchainImage->destroy();
                V3D_DELETE(swapchainImage, memory::MemoryLabel::MemoryRenderCore);
            }
            else
            {
                m_swapchainImages.push_back(swapchainImage);
            }
        }
    }
    else
    {
        ASSERT(m_swapchainImages.size() == swapChainImageCount, "different size");
        std::vector<VkImage>::const_iterator imageIter = images.cbegin();
        for (auto& swapchainImage : m_swapchainImages)
        {
            ASSERT(imageIter != images.cend(), "wrong iterrator");
            if (!swapchainImage->create(*imageIter))
            {
                LOG_FATAL("VulkanSwapchain::createSwapchainImages: can't recreate surface image");
                for (auto& deletedSwapchainImage : m_swapchainImages)
                {
                    deletedSwapchainImage->destroy();
                    V3D_DELETE(deletedSwapchainImage, memory::MemoryLabel::MemoryRenderCore);
                }
                m_swapchainImages.clear();

                return false;
            }
            imageIter = std::next(imageIter);
        }
    }

    return true;
}

void VulkanSwapchain::destroy()
{
    for (auto& swapchainImage : m_swapchainImages)
    {
        swapchainImage->destroy();
        V3D_DELETE(swapchainImage, memory::MemoryLabel::MemoryRenderCore);
    }
    m_swapchainImages.clear();

    if (m_swapchain)
    {
        VulkanWrapper::DestroySwapchain(m_device.getDeviceInfo()._device, m_swapchain, VULKAN_ALLOCATOR);
        LOG_DEBUG("VulkanSwapchain::destroy swapchain destroyed");
        m_swapchain = VK_NULL_HANDLE;
    }

    for (auto& semaphore : m_acquiredSemaphores)
    {
        m_semaphoreManager->deleteSemaphore(semaphore);
    }
    m_acquiredSemaphores.clear();

    if (m_surface)
    {
        VulkanWrapper::DestroySurface(m_device.getDeviceInfo()._instance, m_surface, VULKAN_ALLOCATOR);
        LOG_DEBUG("VulkanSwapchain::destroy surface destroyed");
        m_surface = VK_NULL_HANDLE;
    }

    m_swapchainResources.clear();

    Swapchain::destroy();

    m_ready = false;
}

void VulkanSwapchain::present(VkQueue queue, const std::vector<VulkanSemaphore*>& waitSemaphores)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanSwapchain::present");
#endif //VULKAN_DEBUG
    ASSERT(m_swapchain, "m_swapchain is nullptr");

    VkResult innerResults[1] = {};

    static thread_local std::vector<VkSemaphore> internalWaitSemaphores;
    internalWaitSemaphores.clear();
    internalWaitSemaphores.reserve(waitSemaphores.size());

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
#if !SWAPCHAIN_ON_ADVANCE
    VkSemaphore acquireSemaphore = m_acquiredSemaphores[m_currentSemaphoreIndex]->getHandle();
    ASSERT(m_currentImageIndex == std::get<0>(m_presentInfo) && m_acquiredSemaphores[m_currentSemaphoreIndex] == std::get<1>(m_presentInfo), ("wrong present"));
    if (waitSemaphores.empty())
    {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &acquireSemaphore;
    }
    else
#endif
    {
        for (VulkanSemaphore* semaphore : waitSemaphores)
        {
            internalWaitSemaphores.push_back(semaphore->getHandle());
            m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignToWaiting);
        }

        presentInfo.waitSemaphoreCount = static_cast<u32>(internalWaitSemaphores.size());
        presentInfo.pWaitSemaphores = internalWaitSemaphores.data();
    }
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &std::get<0>(m_presentInfo);
    presentInfo.pResults = innerResults;

    VkResult result = VulkanWrapper::QueuePresent(queue, &presentInfo);
    if (result == VK_ERROR_SURFACE_LOST_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanWrapper::DeviceWaitIdle(m_device.getDeviceInfo()._device);
#ifdef PLATFORM_ANDROID
    ASSERT(g_nativeAndroidApp->window, "windows is nullptr");
#endif //ANDROID_PLATFORM
        //recreate
        LOG_WARNING("VulkanSwapchain::present: Swapchain need to recreate. Error: %s", ErrorString(result).c_str());
        if (!VulkanSwapchain::recteate(m_window, m_params))
        {
            LOG_FATAL(" VulkanSwapchain::QueuePresent: recteate was failed");
        }

        recreateAttachedResources();
    }
    else if (result == VK_SUBOPTIMAL_KHR)
    {
        //Android: One of problem is no used pre-transform
        LOG_WARNING("VulkanSwapchain::present: Swapchain Error: VK_SUBOPTIMAL_KHR");
    }
    else if (result != VK_SUCCESS)
    {
        LOG_FATAL(" VulkanSwapchain::QueuePresent: failed with error %s", ErrorString(result).c_str());
        ASSERT(false, "QueuePresent failed");
    }

#if SWAPCHAIN_ON_ADVANCE
    m_presentInfo = { VulkanSwapchain::currentSwapchainIndex(), m_acquireSemaphore[m_currentSemaphoreIndex] };
#endif
    m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % static_cast<u32>(m_acquiredSemaphores.size());
}

u32 VulkanSwapchain::acquireImage()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanSwapchain::acquireImage semaphoreIndex: %u", m_currentSemaphoreIndex);
#endif //VULKAN_DEBUG
    VulkanSemaphore* semaphore = m_acquiredSemaphores[m_currentSemaphoreIndex];
    m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignToSignal);
    VkFence fence = VK_NULL_HANDLE;

#if DEBUG_FENCE_ACQUIRE
    VkFenceCreateInfo fenceCreateInfo = {};
    {
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;
        VkResult result = VulkanWrapper::CreateFence(m_device.getDeviceInfo()._device, &fenceCreateInfo, VULKAN_ALLOCATOR, &fence);
        ASSERT(result == VK_SUCCESS, "error");
    }
#endif
    
    u32 imageIndex = 0;
    VkResult result = VulkanWrapper::AcquireNextImage(m_device.getDeviceInfo()._device, m_swapchain, UINT64_MAX, semaphore->getHandle(), fence, &imageIndex);

#if DEBUG_FENCE_ACQUIRE
    {
        VkResult result = VulkanWrapper::WaitForFences(m_device.getDeviceInfo()._device, 1, &fence, VK_TRUE, u64(~0ULL));
        ASSERT(result == VK_SUCCESS, "wrong");
        if (result == VK_SUCCESS)
        {
            VulkanWrapper::DestroyFence(m_device.getDeviceInfo()._device, fence, VULKAN_ALLOCATOR);
        }
        else
        {
            ASSERT(false, "fence");
        }
    }
#endif

    if (result == VK_ERROR_SURFACE_LOST_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanWrapper::DeviceWaitIdle(m_device.getDeviceInfo()._device);
#ifdef PLATFORM_ANDROID
    ASSERT(g_nativeAndroidApp->window, "windows is nullptr");
#endif //ANDROID_PLATFORM

        //recreate
        LOG_WARNING("VulkanSwapchain::acquireImage: Swapchain need to recreate. Error: %s", ErrorString(result).c_str());
        if (!VulkanSwapchain::recteate(m_window, m_params))
        {
            LOG_FATAL("VulkanSwapchain::AcquireNextImage: recteate was failed");
        }
        recreateAttachedResources();

        VkResult resultInner = VulkanWrapper::AcquireNextImage(m_device.getDeviceInfo()._device, m_swapchain, UINT64_MAX, semaphore->getHandle(), VK_NULL_HANDLE, &imageIndex);
        if (resultInner != VK_SUCCESS)
        {
            LOG_FATAL(" VulkanSwapchain::AcquireNextImage: failed recreate with error %s", ErrorString(result).c_str());
            ASSERT(false, "vkAcquireNextImageKHR failed recreate");
        }
    }
    else if (result == VK_SUBOPTIMAL_KHR)
    {
        //Android: One of problem is no used pre-transform
        LOG_WARNING("VulkanSwapchain::acquireImage: Swapchain Error: VK_SUBOPTIMAL_KHR");
    }
    else if (result != VK_SUCCESS)
    {
        LOG_FATAL(" VulkanSwapchain::AcquireNextImage: failed with error %s", ErrorString(result).c_str());
        ASSERT(false, "vkAcquireNextImageKHR failed");
    }

    m_currentImageIndex = imageIndex;
#if !SWAPCHAIN_ON_ADVANCE
    m_presentInfo = { imageIndex, semaphore };
#endif
    return imageIndex;
}

bool VulkanSwapchain::recteate(platform::Window* window, const SwapchainParams& params)
{
    if (!m_ready)
    {
        return false;
    }

    for (auto& image : m_swapchainImages)
    {
        image->destroy();
    }

    VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
    if (m_swapchain)
    {
        std::swap(m_swapchain, oldSwapchain);
    }

    for (auto& semaphore : m_acquiredSemaphores)
    {
        m_semaphoreManager->deleteSemaphore(semaphore);
    }
    m_acquiredSemaphores.clear();

    if (m_surface)
    {
        VulkanWrapper::DestroySurface(m_device.getDeviceInfo()._instance, m_surface, VULKAN_ALLOCATOR);
        m_surface = VK_NULL_HANDLE;
    }
    m_ready = false;


    if (!VulkanSwapchain::create(window, params, VK_NULL_HANDLE/*oldSwapchain*/))
    {
        LOG_FATAL("VulkanSwapchain::recteate: is failed");
        return false;
    }

    if (oldSwapchain)
    {
        VulkanWrapper::DestroySwapchain(m_device.getDeviceInfo()._device, oldSwapchain, VULKAN_ALLOCATOR);
    }

    return true;
}

void VulkanSwapchain::attachResource(VulkanResource* resource, const std::function<bool(VulkanResource*)>& recreator)
{
    m_swapchainResources.emplace_back(resource, recreator);
}

void VulkanSwapchain::attachCmdList(VulkanCmdList* cmdlist)
{
    std::scoped_lock lock(m_mutex);

    auto found = std::find(m_cmdLists.begin(), m_cmdLists.end(), cmdlist);
    if (found == m_cmdLists.end())
    {
        m_cmdLists.push_back(cmdlist);
    }
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
