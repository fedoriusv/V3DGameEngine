#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "Utils/Logger.h"


#ifdef VULKAN_RENDER
//#   ifdef PLATFORM_WINDOWS
//#       pragma comment(lib, "vulkan-1.lib")
//#   endif //PLATFORM_WINDOWS
namespace v3d
{
namespace renderer
{
namespace vk
{
const std::vector<const c8*> k_instanceExtensionsList = 
{
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_DISPLAY_EXTENSION_NAME,
#ifdef VK_KHR_win32_surface
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_KHR_android_surface
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_KHR_xlib_surface
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#if VULKAN_LAYERS_CALLBACKS
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif //VULKAN_LAYERS_CALLBACKS
};

const std::vector<const c8*> k_deviceExtensionsList =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
};

VulkanGraphicContext::VulkanGraphicContext(const platform::Window* window)
    : m_swapchain(nullptr)
    , m_drawCmdBufferManager(nullptr)
    , m_currentDrawBuffer(nullptr)
    , m_window(window)
    , m_frameCounter(0U)
{
    LOG_DEBUG("VulkanGraphicContext created this %llx", this);

    m_renderType = RenderType::VulkanRender;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));
}

VulkanGraphicContext::~VulkanGraphicContext()
 {
    LOG_DEBUG("~VulkanGraphicContext destructor this %llx", this);

    ASSERT(!m_swapchain, "m_swapchain not nullptr");
    ASSERT(!m_drawCmdBufferManager, "m_drawCmdBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");
 }

void VulkanGraphicContext::beginFrame()
{
    u32 index = m_swapchain->acquireImage();
    LOG_DEBUG("VulkanGraphicContext::beginFrame %llu, image index %u", m_frameCounter, index);

    m_currentDrawBuffer = m_drawCmdBufferManager->acquireNewCmdBuffer(VulkanCommandBufferManager::CommandTargetType::CmdDrawBuffer);
    ASSERT(m_currentDrawBuffer, "m_currentDrawBuffer is nullptr");

    if (m_currentDrawBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Invalid)
    {
        LOG_ERROR("VulkanGraphicContext::beginFrame CommandBufferStatus is Invalid");
    }

    //TODO: transfer swapchain layout to write
}

void VulkanGraphicContext::endFrame()
{
    LOG_DEBUG("VulkanGraphicContext::endFrame %llu", m_frameCounter);

    //TODO: transfer swapchain layout to present

    std::vector<VkSemaphore> semaphores;
    m_drawCmdBufferManager->submit(m_currentDrawBuffer, semaphores);
}

void VulkanGraphicContext::presentFrame()
{
    LOG_DEBUG("VulkanGraphicContext::presentFrame %llu", m_frameCounter);

    std::vector<VkSemaphore> semaphores;
    m_swapchain->present(m_queueList[0], semaphores);

    m_frameCounter++;
}

void VulkanGraphicContext::clearColor(const core::Vector4D & color)
{
    LOG_DEBUG("VulkanGraphicContext::clearColor [%u, %u, %u, %u]", color[0], color[1], color[2], color[3]);
   // m_currentDrawBuffer->CmdClearColor();
}

void VulkanGraphicContext::setViewport(const core::Rect32& viewport)
{
    LOG_DEBUG("VulkanGraphicContext::setViewport [%u, %u; %u, %u]", viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
    //TODO:
}

Image * VulkanGraphicContext::createImage(TextureTarget target, renderer::ImageFormat format, core::Dimension3D dimension, u32 mipLevels, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    VkImageType vkType = VulkanImage::convertTextureTargetToVkImageType(target);
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };

    //TODO: memory pool
    return new VulkanImage(m_deviceInfo._device, vkType, vkFormat, vkExtent, mipLevels);
}

bool VulkanGraphicContext::initialize()
{
    //Called from game thread
    LOG_DEBUG("VulkanGraphicContext::initialize");
    if (!LoadVulkanLibrary())
    {
        LOG_WARNING("VulkanGraphicContext::initialize: LoadVulkanLibrary is falied");
    }

    if (!VulkanGraphicContext::createInstance())
    {
        LOG_FATAL("VulkanGraphicContext::createInstance failed");

        ASSERT(false, "createInstance failed");
        return false;
    }

    if (!VulkanGraphicContext::createDevice())
    {
        LOG_FATAL("VulkanGraphicContext::createDevice failed");
        VulkanGraphicContext::destroy();

        ASSERT(false, "createDevice failed");
        return false;
    }

    LOG_INFO("VulkanGraphicContext::initialize count Queue %u", m_queueList.size());
    for (u32 queueIndex = 0; queueIndex < m_queueList.size(); ++queueIndex)
    {
        VulkanWrapper::GetDeviceQueue(m_deviceInfo._device, m_deviceInfo._queueFamilyIndex, queueIndex, &m_queueList[queueIndex]);
    }

    VkSurfaceKHR surface = VulkanSwapchain::createSurface(m_deviceInfo._instance, m_window->getInstance(), m_window->getWindowHandle());
    if (!surface)
    {
        VulkanGraphicContext::destroy();
        LOG_FATAL("VulkanGraphicContext::createContext: Can not create VkSurfaceKHR");
        return false;
    }

    VulkanSwapchain::SwapchainConfig config;
    config._size = m_window->getSize();
    config._vsync = true; //TODO

    m_swapchain = new VulkanSwapchain(&m_deviceInfo, surface);
    if (!m_swapchain->create(config))
    {
        VulkanGraphicContext::destroy();
        LOG_FATAL("VulkanGraphicContext::createContext: Can not create VulkanSwapchain");
        return false;
    }

    m_drawCmdBufferManager = new VulkanCommandBufferManager(&m_deviceInfo, m_queueList[0]);
    m_currentDrawBuffer = m_drawCmdBufferManager->acquireNewCmdBuffer(VulkanCommandBufferManager::CommandTargetType::CmdDrawBuffer);
    //ASSERT(m_currentDrawBuffer, "m_currentDrawBuffer is nullptr");

    return true;
}

void VulkanGraphicContext::destroy()
{
    //Called from game thread
    LOG_DEBUG("VulkanGraphicContext::destroy");

    //TODO: Wait to complete all commands
    if (m_drawCmdBufferManager)
    {
        m_drawCmdBufferManager->waitCompete();

        delete m_drawCmdBufferManager;
        m_drawCmdBufferManager = nullptr;
    }

    if (m_swapchain)
    {
        m_swapchain->destroy();
        VulkanSwapchain::detroySurface(m_deviceInfo._instance, m_swapchain->m_surface);

        delete m_swapchain;
        m_swapchain = nullptr;
    }

    if (m_deviceInfo._device)
    {
        VulkanWrapper::DestroyDevice(m_deviceInfo._device, VULKAN_ALLOCATOR);
        m_deviceInfo._device = VK_NULL_HANDLE;
        m_queueList.clear();
    }

#if VULKAN_LAYERS_CALLBACKS
    VulkanDebug::destroyDebugUtilsMesseger(m_deviceInfo._instance);
#endif //VULKAN_LAYERS_CALLBACKS

    if (m_deviceInfo._instance)
    {
        VulkanWrapper::DestroyInstance(m_deviceInfo._instance, VULKAN_ALLOCATOR);
        m_deviceInfo._instance = VK_NULL_HANDLE;
    }
}

bool VulkanGraphicContext::createInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = s_vulkanApplicationName.c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = s_vulkanApplicationName.c_str();
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(VULKAN_VERSION_MAJOR, VULKAN_VERSION_MINOR, VULKAN_VERSION_PATCH);

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfInstanceExtensions(supportedExtensions);

#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanGraphicContext::createInstance: extention: [%s]", (*iter).c_str());
    }
#endif //VULKAN_DEBUG
    std::vector<const c8*> enabledExtensions;
    for (auto extentionName = k_instanceExtensionsList.cbegin(); extentionName != k_instanceExtensionsList.cend(); ++extentionName)
    {
        bool found = false;
        for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
        {
            if (!(*iter).compare(*extentionName))
            {
                LOG_INFO("VulkanGraphicContext::createInstance: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanGraphicContext::createInstance: extention [%s] is not supported", *extentionName);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanDebug::s_validationLayerNames.cbegin(); layerName < VulkanDebug::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanDebug::checkInstanceLayerIsSupported(*layerName))
        {
            LOG_INFO("VulkanGraphicContext::createInstance: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

#if VULKAN_RENDERDOC_LAYER
    const c8* renderdocLayerName = "VK_LAYER_RENDERDOC_Capture";
    if (VulkanDebug::checkLayerIsSupported(renderdocLayerName))
    {
        LOG_INFO("VulkanGraphicContext::createInstance: enable layer: [%s]", renderdocLayerName);
        layerNames.push_back(renderdocLayerName);
    }
#endif //VULKAN_RENDERDOC_LAYER

    instanceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

    VkResult result = VulkanWrapper::CreateInstance(&instanceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._instance);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanGraphicContext::createInstance: vkCreateInstance error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._instance))
    {
        LOG_WARNING("VulkanGraphicContext::createInstance: LoadVulkanLibrary is falied");
    }

#if VULKAN_LAYERS_CALLBACKS
    VkDebugUtilsMessageSeverityFlagsEXT severityFlag = 0;
    switch (VulkanDebug::s_severityDebugLevel)
    {
    case 4:
        severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    case 3:
        severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    case 2:
        severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    case 1:
        severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    case 0:
    default:
        //turn off
        break;
    }

    VkDebugUtilsMessageTypeFlagsEXT messageTypeFlag = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    messageTypeFlag |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

    if(!VulkanDebug::createDebugUtilsMesseger(m_deviceInfo._instance, severityFlag, messageTypeFlag, nullptr, this))
    {
        LOG_ERROR("VulkanGraphicContext::createInstance: createDebugUtilsMessager failed");
    }
#endif //VULKAN_LAYERS_CALLBACKS

    return true;
}

bool VulkanGraphicContext::createDevice()
{
    ASSERT(m_deviceInfo._instance != VK_NULL_HANDLE, "instance is nullptr");

    // Physical device
    u32 gpuCount = 0;
    VkResult result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanGraphicContext::createDevice: Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_INFO("VulkanGraphicContext::createDevice: count GPU: %u", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, physicalDevices.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanGraphicContext::createDevice: Can not enumerate phyiscal devices. vkEnumeratePhysicalDevices Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_INFO("VulkanGraphicContext::createDevice: count GPU: %u, use first", gpuCount);
    m_deviceInfo._physicalDevice = physicalDevices.front();

    m_deviceCaps.fillCapabilitiesList(&m_deviceInfo);

    u32 familyIndex = m_deviceCaps.getQueueFamiliyIndex(VK_QUEUE_GRAPHICS_BIT);
    //index family, queue bits, priority list 
    const std::list<std::tuple<u32, VkQueueFlags, std::vector<f32>>> queueLists =
    {
        { familyIndex, VK_QUEUE_GRAPHICS_BIT,{ 0.0f } },
    };
    m_queueList.resize(queueLists.size(), VK_NULL_HANDLE);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto& requestedQueue : queueLists)
    {
        s32 requestedQueueFamalyIndex = std::get<0>(requestedQueue);
        VkQueueFlags requestedQueueTypes = std::get<1>(requestedQueue);
        const std::vector<f32>& queuePriority = std::get<2>(requestedQueue);
        s32 requestedQueueCount = static_cast<s32>(std::get<2>(requestedQueue).size());

        m_deviceInfo._queueFamilyIndex = requestedQueueFamalyIndex;
        //m_queueFlag = m_deviceCaps.m_queueFamilyProperties[requestedQueueFamalyIndex].queueFlags;

        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext = nullptr;
        queueInfo.flags = 0;
        queueInfo.queueFamilyIndex = requestedQueueFamalyIndex;
        queueInfo.queueCount = requestedQueueCount;
        queueInfo.pQueuePriorities = queuePriority.data();

        queueCreateInfos.push_back(queueInfo);
    }

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfDeviceExtensions(m_deviceInfo._physicalDevice, supportedExtensions);
#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanGraphicContext::createDevice: extention: [%s]", (*iter).c_str());
    }
#endif //VULKAN_DEBUG
    std::vector<const c8*> enabledExtensions;
    for (auto extentionName = k_deviceExtensionsList.cbegin(); extentionName != k_deviceExtensionsList.cend(); ++extentionName)
    {
        bool found = false;
        for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
        {
            if (!(*iter).compare(*extentionName))
            {
                LOG_INFO("VulkanGraphicContext::createDevice: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanGraphicContext::createDevice: extention [%s] is not supported", *extentionName);
        }
    }

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &m_deviceCaps.m_deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanDebug::s_validationLayerNames.cbegin(); layerName < VulkanDebug::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanDebug::checkDeviceLayerIsSupported(m_deviceInfo._physicalDevice, *layerName))
        {
            LOG_INFO("VulkanGraphicContext::createDevice: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK
    deviceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    deviceCreateInfo.ppEnabledLayerNames = layerNames.data();

    result = VulkanWrapper::CreateDevice(m_deviceInfo._physicalDevice, &deviceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanGraphicContext::createDevice: vkCreateDevice Error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._device))
    {
        LOG_WARNING("VulkanGraphicContext::createDevice: LoadVulkanLibrary is falied");
    }

    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
