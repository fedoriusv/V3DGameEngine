#include "VulkanContext.h"

#include "Utils/Logger.h"
#include "Utils/Profiler.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanBuffer.h"
#include "VulkanStagingBuffer.h"
#include "VulkanUnifromBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanComputePipeline.h"
#include "VulkanContextState.h"
#include "VulkanSemaphore.h"

#include "Renderer/FrameTimeProfiler.h"
#ifdef PLATFORM_ANDROID
#   include "Platform/Android/HWCPProfiler.h"
#endif //PLATFORM_ANDROID

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
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
#endif //VULKAN_LAYERS_CALLBACKS

    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
};

const std::vector<const c8*> k_deviceExtensionsList =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,

    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,

    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_MAINTENANCE2_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,

    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,

    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_MULTIVIEW_EXTENSION_NAME,

    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

    VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME,

    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
#ifdef VK_EXT_custom_border_color
    VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
#endif
#ifdef VK_QCOM_render_pass_transform
    VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME,
#endif
#ifdef VK_EXT_astc_decode_mode
    VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME,
#endif
};

std::vector<VkDynamicState> VulkanContext::s_dynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    //VK_DYNAMIC_STATE_DEPTH_BIAS,
    //VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    //VK_DYNAMIC_STATE_STENCIL_REFERENCE
};

VulkanContext::VulkanContext(platform::Window* window, DeviceMask mask) noexcept
    : m_deviceCaps(*VulkanDeviceCaps::getInstance())
    , m_swapchain(nullptr)
    , m_cmdBufferManager(nullptr)

    , m_pipelineLayoutManager(nullptr)
    , m_descriptorSetManager(nullptr)
    , m_stagingBufferManager(nullptr)
    , m_uniformBufferManager(nullptr)
    , m_semaphoreManager(nullptr)

    , m_renderpassManager(nullptr)
    , m_framebufferManager(nullptr)
    , m_pipelineManager(nullptr)
    , m_samplerManager(nullptr)

    , m_imageMemoryManager(nullptr)
    , m_bufferMemoryManager(nullptr)

    , m_currentContextState(nullptr)

    , m_insideFrame(false)
    , m_window(window)
{
    LOG_DEBUG("VulkanContext created this %llx", this);

    m_renderType = RenderType::VulkanRender;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));
    m_deviceInfo._mask = mask;

#if VULKAN_DUMP
    VulkanDump::getInstance()->init(VulkanDump::DumpFlag_None);
#endif
}

VulkanContext::~VulkanContext()
{
    LOG_DEBUG("~VulkanContext destructor this %llx", this);

    ASSERT(!m_imageMemoryManager, "m_imageMemoryManager not nullptr");
    ASSERT(!m_bufferMemoryManager, "m_bufferMemoryManager not nullptr");
    ASSERT(!m_stagingBufferManager, "m_stagingBufferManager not nullptr");

    ASSERT(!m_pipelineLayoutManager, "m_pipelineLayoutManager not nullptr");
    ASSERT(!m_descriptorSetManager, "m_descriptorSetManager not nullptr");
    ASSERT(!m_stagingBufferManager, "m_stagingBufferManager not nullptr");
    ASSERT(!m_uniformBufferManager, "m_uniformBufferManager not nullptr");
    ASSERT(!m_semaphoreManager, "m_semaphoreManager not nullptr");

    ASSERT(!m_renderpassManager, "m_renderpassManager not nullptr");
    ASSERT(!m_framebufferManager, "m_framebufferManager not nullptr");
    ASSERT(!m_pipelineManager, "m_pipelineManager not nullptr");
    ASSERT(!m_samplerManager, "m_samplerManager not nullptr");

    ASSERT(!m_swapchain, "m_swapchain not nullptr");
    ASSERT(!m_cmdBufferManager, "m_cmdBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");
}

bool VulkanContext::initialize()
{
    //Called from game thread
    LOG_DEBUG("VulkanContext::initialize");
    if (!LoadVulkanLibrary())
    {
        LOG_WARNING("VulkanContext::initialize: LoadVulkanLibrary is falied");
}

    if (!VulkanContext::createInstance())
    {
        LOG_FATAL("VulkanContext::createInstance is failed");

        ASSERT(false, "createInstance is failed");
        return false;
    }

    if (!VulkanContext::createDevice())
    {
        LOG_FATAL("VulkanContext::createDevice is failed");
        VulkanContext::destroy();

        ASSERT(false, "createDevice is failed");
        return false;
    }

    LOG_INFO("VulkanContext::initialize count Queue %u", m_queueList.size());
    for (u32 queueIndex = 0; queueIndex < m_queueList.size(); ++queueIndex)
    {
        VulkanWrapper::GetDeviceQueue(m_deviceInfo._device, m_deviceInfo._queueFamilyIndex, queueIndex, &m_queueList[queueIndex]);
    }
    m_deviceCaps.initialize();

    VulkanSwapchain::SwapchainConfig config;
    config._window = m_window;
    config._size = m_window->getSize();
    config._forceSRGB = false;
#ifdef PLATFORM_ANDROID
    config._vsync = true;
    config._countSwapchainImages = 3;
#else
    config._vsync = false; //TODO need config from high level
    config._countSwapchainImages = 3;
#endif

    m_semaphoreManager = new VulkanSemaphoreManager(m_deviceInfo._device);
    m_swapchain = new VulkanSwapchain(&m_deviceInfo, m_semaphoreManager);
    if (!m_swapchain->create(config))
    {
        VulkanContext::destroy();
        LOG_FATAL("VulkanContext::createContext: Can not create VulkanSwapchain");
        return false;
    }

    m_window->registerNotify(this);
#if SWAPCHAIN_ON_ADVANCE
    m_presentSemaphores.push_back(m_swapchain->getAcquireSemaphore(0));
#endif

#if THREADED_PRESENT
    m_presentThread = new PresentThread(m_swapchain);
#endif //THREADED_PRESENT

    m_backufferDescription._size = { m_swapchain->getSwapchainImage(0)->getSize().width, m_swapchain->getSwapchainImage(0)->getSize().height };
    m_backufferDescription._format = VulkanImage::convertVkImageFormatToFormat(m_swapchain->getSwapchainImage(0)->getFormat());

    if (m_deviceCaps.unifiedMemoryManager)
    {
        m_imageMemoryManager = new SimpleVulkanMemoryAllocator(m_deviceInfo._device);
        m_bufferMemoryManager = m_imageMemoryManager;
    }
    else
    {
        m_imageMemoryManager = new PoolVulkanMemoryAllocator(m_deviceInfo._device, 64 * 1024 * 1024); //64MB
        m_bufferMemoryManager = new PoolVulkanMemoryAllocator(m_deviceInfo._device, 4 * 1024 * 1024); //4MB
    }

    m_cmdBufferManager = new VulkanCommandBufferManager(this, &m_deviceInfo, m_semaphoreManager, m_queueList[0]);
    m_currentBufferState._commandBufferMgr = m_cmdBufferManager;

    m_stagingBufferManager = new VulkanStagingBufferManager(m_deviceInfo._device);
    m_uniformBufferManager = new VulkanUniformBufferManager(m_deviceInfo._device, m_resourceDeleter);
    m_pipelineLayoutManager = new VulkanPipelineLayoutManager(m_deviceInfo._device);
    m_descriptorSetManager = new VulkanDescriptorSetManager(m_deviceInfo._device, m_swapchain->getSwapchainImageCount());

    m_renderpassManager = new RenderPassManager(this);
    m_framebufferManager = new FramebufferManager(this);
    m_pipelineManager = new PipelineManager(this);
    m_samplerManager = new SamplerManager(this);

    m_currentContextState = new VulkanContextState(m_deviceInfo._device, m_descriptorSetManager, m_uniformBufferManager);

#if FRAME_PROFILER_ENABLE
    utils::ProfileManager::getInstance()->attach(new FrameTimeProfiler());
#   if defined(PLATFORM_ANDROID)
    utils::ProfileManager::getInstance()->attach(new android::HWCPProfiler(
        {
            android::HWCPProfiler::CpuCounter::Cycles,
            android::HWCPProfiler::CpuCounter::Instructions,
            android::HWCPProfiler::CpuCounter::CacheReferences,
            android::HWCPProfiler::CpuCounter::CacheMisses,
            android::HWCPProfiler::CpuCounter::BranchInstructions,
            android::HWCPProfiler::CpuCounter::BranchMisses,
            android::HWCPProfiler::CpuCounter::L1Accesses,
            android::HWCPProfiler::CpuCounter::InstrRetired,
            android::HWCPProfiler::CpuCounter::L2Accesses,
            android::HWCPProfiler::CpuCounter::L3Accesses,
            android::HWCPProfiler::CpuCounter::BusReads,
            android::HWCPProfiler::CpuCounter::BusWrites,
            android::HWCPProfiler::CpuCounter::MemReads,
            android::HWCPProfiler::CpuCounter::MemWrites,
            android::HWCPProfiler::CpuCounter::ASESpec,
            android::HWCPProfiler::CpuCounter::VFPSpec,
            android::HWCPProfiler::CpuCounter::CryptoSpec
        },
        {
            android::HWCPProfiler::GpuCounter::GpuCycles,
            android::HWCPProfiler::GpuCounter::VertexComputeCycles,
            android::HWCPProfiler::GpuCounter::FragmentCycles,
            android::HWCPProfiler::GpuCounter::TilerCycles,
            android::HWCPProfiler::GpuCounter::VertexComputeJobs,
            android::HWCPProfiler::GpuCounter::FragmentJobs,
            android::HWCPProfiler::GpuCounter::Pixels,
            android::HWCPProfiler::GpuCounter::Tiles,
            android::HWCPProfiler::GpuCounter::TransactionEliminations,
            android::HWCPProfiler::GpuCounter::EarlyZTests,
            android::HWCPProfiler::GpuCounter::EarlyZKilled,
            android::HWCPProfiler::GpuCounter::LateZTests,
            android::HWCPProfiler::GpuCounter::LateZKilled,
            android::HWCPProfiler::GpuCounter::Instructions,
            android::HWCPProfiler::GpuCounter::DivergedInstructions,
            android::HWCPProfiler::GpuCounter::ShaderCycles,
            android::HWCPProfiler::GpuCounter::ShaderArithmeticCycles,
            android::HWCPProfiler::GpuCounter::ShaderLoadStoreCycles,
            android::HWCPProfiler::GpuCounter::ShaderTextureCycles,
            android::HWCPProfiler::GpuCounter::CacheReadLookups,
            android::HWCPProfiler::GpuCounter::CacheWriteLookups,
            android::HWCPProfiler::GpuCounter::ExternalMemoryReadAccesses,
            android::HWCPProfiler::GpuCounter::ExternalMemoryWriteAccesses,
            android::HWCPProfiler::GpuCounter::ExternalMemoryReadStalls,
            android::HWCPProfiler::GpuCounter::ExternalMemoryWriteStalls,
            android::HWCPProfiler::GpuCounter::ExternalMemoryReadBytes,
            android::HWCPProfiler::GpuCounter::ExternalMemoryWriteBytes
        }));
#   endif //PLATFORM_ANDROID
#endif //FRAME_PROFILER_ENABLE
    return true;
}

bool VulkanContext::createInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = s_vulkanApplicationName.c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = s_vulkanApplicationName.c_str();
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(VULKAN_VERSION_MAJOR, VULKAN_VERSION_MINOR, 0);

    {
        u32 apiVersion = 0;
        VkResult result = VulkanWrapper::EnumerateInstanceVersion(&apiVersion);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanContext::createInstance: vkEnumerateInstanceVersion error %s", ErrorString(result).c_str());
        }
        else
        {
            LOG_INFO("Requested Vulkan Api: %u (%u.%u.%u), supported: %u (%u.%u.%u)", applicationInfo.apiVersion, VK_VERSION_MAJOR(applicationInfo.apiVersion), VK_VERSION_MINOR(applicationInfo.apiVersion), VK_VERSION_PATCH(applicationInfo.apiVersion),
                apiVersion, VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));
        }

    }

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfInstanceExtensions(supportedExtensions);

#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanContext::createInstance: extention: [%s]", (*iter).c_str());
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
                LOG_INFO("VulkanContext::createInstance: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanContext::createInstance: extention [%s] is not supported", *extentionName);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr; //VkDebugUtilsMessengerCreateInfoEXT
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanLayers::s_validationLayerNames.cbegin(); layerName < VulkanLayers::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanLayers::checkInstanceLayerIsSupported(*layerName))
        {
            LOG_INFO("VulkanContext::createInstance: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

#if VULKAN_RENDERDOC_LAYER
    const c8* renderdocLayerName = "VK_LAYER_RENDERDOC_Capture";
    if (VulkanLayers::checkLayerIsSupported(renderdocLayerName))
    {
        LOG_INFO("VulkanContext::createInstance: enable layer: [%s]", renderdocLayerName);
        layerNames.push_back(renderdocLayerName);
    }
#endif //VULKAN_RENDERDOC_LAYER

    instanceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

    VkResult result = VulkanWrapper::CreateInstance(&instanceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._instance);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanContext::createInstance: vkCreateInstance error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._instance))
    {
        LOG_WARNING("VulkanContext::createInstance: LoadVulkanLibrary is falied");
    }

#if VULKAN_LAYERS_CALLBACKS
    if (VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        VkDebugUtilsMessageSeverityFlagsEXT severityFlag = 0;
        switch (VulkanDebugUtils::k_severityDebugLevel)
        {
        case 4:
            severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            [[fallthrough]];
        case 3:
            severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            [[fallthrough]];
        case 2:
            severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            [[fallthrough]];
        case 1:
            severityFlag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            [[fallthrough]];
        case 0:
        default:
            //turn off
            break;
        }

        VkDebugUtilsMessageTypeFlagsEXT messageTypeFlag = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
#   if VULKAN_VALIDATION_LAYERS_CALLBACK
        messageTypeFlag |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
#   endif //VULKAN_VALIDATION_LAYERS_CALLBACK
        if (!VulkanDebugUtils::createDebugUtilsMessenger(m_deviceInfo._instance, severityFlag, messageTypeFlag, nullptr, this))
        {
            LOG_ERROR("VulkanContext::createInstance: createDebugUtilsMessager failed");
        }
    }
    else if (VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        VkDebugReportFlagsEXT flags = 0;
#   if VULKAN_VALIDATION_LAYERS_CALLBACK
        switch (VulkanDebugReport::k_debugReportLevel)
        {
        case 4:
            flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
            [[fallthrough]];
        case 3:
            flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
            [[fallthrough]];
        case 2:
            flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            [[fallthrough]];
        case 1:
            flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
            [[fallthrough]];
        case 0:
        default:
            //turn off
            break;
        }
#   else
        flags |= (VulkanDebugReport::k_debugReportLevel == 4) ? VK_DEBUG_REPORT_DEBUG_BIT_EXT : 0;
#   endif //VULKAN_VALIDATION_LAYERS_CALLBACK

        if (!VulkanDebugReport::createDebugReportCallback(m_deviceInfo._instance, flags, nullptr, this))
        {
            LOG_ERROR("VulkanContext::createInstance: createDebugReportCallback failed");
        }
    }
#endif //VULKAN_LAYERS_CALLBACKS
    return true;
}

bool VulkanContext::createDevice()
{
    ASSERT(m_deviceInfo._instance != VK_NULL_HANDLE, "instance is nullptr");

    // Physical device
    u32 gpuCount = 0;
    VkResult result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanContext::createDevice: Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_INFO("VulkanContext::createDevice: count GPU: %u", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, physicalDevices.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanContext::createDevice: Can not enumerate phyiscal devices. vkEnumeratePhysicalDevices Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_INFO("VulkanContext::createDevice: count GPU: %u, use first", gpuCount);
    m_deviceInfo._physicalDevice = physicalDevices.front();

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfDeviceExtensions(m_deviceInfo._physicalDevice, supportedExtensions);
#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanContext::createDevice: extention: [%s]", (*iter).c_str());
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
                LOG_INFO("VulkanContext::createDevice: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanContext::createDevice: extention [%s] is not supported", *extentionName);
        }
    }
    VulkanDeviceCaps::s_enableExtensions = enabledExtensions;

    m_deviceCaps.fillCapabilitiesList(&m_deviceInfo);

    ASSERT(m_deviceInfo._mask, "is empty");
    u32 familyIndex = m_deviceCaps.getQueueFamiliyIndex(m_deviceInfo._mask);
    //index family, queue bits, priority list 
    const std::list<std::tuple<u32, VkQueueFlags, std::vector<f32>>> queueLists =
    {
        { familyIndex, m_deviceInfo._mask, { 0.0f } },
    };
    m_queueList.resize(queueLists.size(), VK_NULL_HANDLE);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto& requestedQueue : queueLists)
    {
        s32 requestedQueueFamalyIndex = std::get<0>(requestedQueue);
        [[maybe_unused]] VkQueueFlags requestedQueueTypes = std::get<1>(requestedQueue);
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

    void* vkExtension = nullptr;
#if VULKAN_VALIDATION_LAYERS_CALLBACK //needs for validations check
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
    {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT& physicalDeviceDescriptorIndexingFeatures = VulkanDeviceCaps::getInstance()->m_physicalDeviceDescriptorIndexingFeatures;
        physicalDeviceDescriptorIndexingFeatures.pNext = vkExtension;
        vkExtension = &physicalDeviceDescriptorIndexingFeatures;
    }

#ifdef VK_EXT_custom_border_color
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
    {
        VkPhysicalDeviceCustomBorderColorFeaturesEXT& physicalDeviceCustomBorderColorFeatures = VulkanDeviceCaps::getInstance()->m_physicalDeviceCustomBorderColorFeatures;
        physicalDeviceCustomBorderColorFeatures.pNext = vkExtension;
        vkExtension = &physicalDeviceCustomBorderColorFeatures;
    }
#endif

    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
    {
        VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR physicalDevicePipelineExecutablePropertiesFeatures = {};
        physicalDevicePipelineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
        physicalDevicePipelineExecutablePropertiesFeatures.pNext = nullptr;
        physicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo = VulkanDeviceCaps::getInstance()->pipelineExecutablePropertiesEnabled;
        vkExtension = &physicalDevicePipelineExecutablePropertiesFeatures;
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = vkExtension;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &m_deviceCaps.m_deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanLayers::s_validationLayerNames.cbegin(); layerName < VulkanLayers::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanLayers::checkDeviceLayerIsSupported(m_deviceInfo._physicalDevice, *layerName))
        {
            LOG_INFO("VulkanContext::createDevice: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK
    deviceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    deviceCreateInfo.ppEnabledLayerNames = layerNames.data();

    result = VulkanWrapper::CreateDevice(m_deviceInfo._physicalDevice, &deviceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanContext::createDevice: vkCreateDevice Error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._device))
    {
        LOG_WARNING("VulkanContext::createDevice: LoadVulkanLibrary is falied");
    }

    return true;
}

void VulkanContext::destroy()
{
    //Called from game thread
    LOG_DEBUG("VulkanContext::destroy");
    const_cast<platform::Window*>(m_window)->unregisterNotify(this);

    VkResult result = VulkanWrapper::DeviceWaitIdle(m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanContext::destroy DeviceWaitIdle is failed. Error: %s", ErrorString(result).c_str());
        ASSERT(false, "error");
    }

#if FRAME_PROFILER_ENABLE
    utils::ProfileManager::getInstance()->freeAllProfilers();
#endif //FRAME_PROFILER_ENABLE

#if THREADED_PRESENT
    delete m_presentThread;
#endif //THREADED_PRESENT
    //reset to max, need to skip all safe frames
    m_frameCounter = ~0U;
    if (m_cmdBufferManager)
    {
        m_cmdBufferManager->waitCompete();

        delete m_cmdBufferManager;
        m_cmdBufferManager = nullptr;
    }
    m_resourceDeleter.updateResourceDeleter(true);

    if (m_stagingBufferManager)
    {
        delete m_stagingBufferManager;
        m_stagingBufferManager = nullptr;
    }

    if (m_uniformBufferManager)
    {
        m_uniformBufferManager->updateUniformBuffers();

        delete m_uniformBufferManager;
        m_uniformBufferManager = nullptr;
    }

    if (m_deviceCaps.unifiedMemoryManager)
    {
        delete m_imageMemoryManager;
        m_imageMemoryManager = nullptr;
        m_bufferMemoryManager = nullptr;
    }
    else
    {
        delete m_imageMemoryManager;
        m_imageMemoryManager = nullptr;

        delete m_bufferMemoryManager;
        m_bufferMemoryManager = nullptr;
    }

    if (m_pipelineManager)
    {
        delete m_pipelineManager;
        m_pipelineManager = nullptr;
    }

    if (m_descriptorSetManager)
    {
        m_descriptorSetManager->updateDescriptorPools();

        delete m_descriptorSetManager;
        m_descriptorSetManager = nullptr;
    }

    if (m_pipelineLayoutManager)
    {
        m_pipelineLayoutManager->clear();

        delete m_pipelineLayoutManager;
        m_pipelineLayoutManager = nullptr;
    }

    if (m_renderpassManager)
    {
        delete m_renderpassManager;
        m_renderpassManager = nullptr;
    }

    if (m_framebufferManager)
    {
        delete m_framebufferManager;
        m_framebufferManager = nullptr;
    }

    if (m_samplerManager)
    {
        delete m_samplerManager;
        m_samplerManager = nullptr;
    }

    if (m_currentContextState)
    {
        delete m_currentContextState;
        m_currentContextState = nullptr;
    }

    if (m_swapchain)
    {
        m_swapchain->destroy();
        delete m_swapchain;
        m_swapchain = nullptr;
    }

    if (m_semaphoreManager)
    {
        m_semaphoreManager->updateSemaphores();
        m_semaphoreManager->clear();
        delete m_semaphoreManager;
        m_semaphoreManager = nullptr;
    }
#if DEBUG_OBJECT_MEMORY
    ASSERT(VulkanBuffer::s_objects.empty(), "buffer objects still exist");
    ASSERT(VulkanImage::s_objects.empty(), "image objects still exist");
#endif //DEBUG_OBJECT_MEMORY

    if (m_deviceInfo._device)
    {
        VulkanWrapper::DestroyDevice(m_deviceInfo._device, VULKAN_ALLOCATOR);
        m_deviceInfo._device = VK_NULL_HANDLE;
        m_queueList.clear();
    }

#if VULKAN_LAYERS_CALLBACKS
    if (VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        VulkanDebugUtils::destroyDebugUtilsMessenger(m_deviceInfo._instance);
    }
    else if (VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        VulkanDebugReport::destroyDebugReportCallback(m_deviceInfo._instance);
    }
#endif //VULKAN_LAYERS_CALLBACKS

    if (m_deviceInfo._instance)
    {
        VulkanWrapper::DestroyInstance(m_deviceInfo._instance, VULKAN_ALLOCATOR);
        m_deviceInfo._instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::beginFrame()
{
    ASSERT(!m_insideFrame, "must be outside");
    m_insideFrame = true;

#if THREADED_PRESENT
    u32 index = 0;
    m_presentThread->requestAcquireImage(index);
#else
    [[maybe_unused]] u32 prevImageIndex = VulkanSwapchain::currentSwapchainIndex();
    [[maybe_unused]] u32 index = m_swapchain->acquireImage();
#endif //THREADED_PRESENT

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::beginFrame %llu, image index %u", m_frameCounter, index);
#endif //VULKAN_DEBUG

#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpFrameNumber(m_frameCounter);
#endif

    ASSERT(!m_currentBufferState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "buffer exist");
    [[maybe_unused]] VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
#if SWAPCHAIN_ON_ADVANCE
    ASSERT(prevImageIndex != ~0U, "wrong index");
    m_currentTransitionState.transitionImages(drawBuffer, { { m_swapchain->getSwapchainImage(prevImageIndex), { 0, 1, 0, 1} } }, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
#endif //SWAPCHAIN_ON_ADVANCE

#if FRAME_PROFILER_ENABLE
    utils::ProfileManager::getInstance()->update();
    utils::ProfileManager::getInstance()->start();
#endif //FRAME_PROFILER_ENABLE
}

void VulkanContext::endFrame()
{
    ASSERT(m_insideFrame, "must be inside");
    m_insideFrame = false;

#if FRAME_PROFILER_ENABLE
    utils::ProfileManager::getInstance()->stop();
#endif //FRAME_PROFILER_ENABLE

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::endFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG
}

void VulkanContext::presentFrame()
{
    ASSERT(!m_insideFrame, "must be outside");

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::presentFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG

    VulkanContext::submit();

    std::vector<VulkanSemaphore*> semaphores;
    semaphores.insert(semaphores.end(), m_waitSemaphores.begin(), m_waitSemaphores.end());
    semaphores.insert(semaphores.end(), m_presentSemaphores.begin(), m_presentSemaphores.end());
    m_waitSemaphores.clear();
    m_presentSemaphores.clear();

#if THREADED_PRESENT
    m_presentThread->requestPresent(m_queueList[0], 0);
#else
    m_swapchain->present(m_queueList[0], semaphores);
#endif //THREADED_PRESENT
    ++m_frameCounter;
}

void VulkanContext::submit(bool wait)
{
    //Uploads commands
    VulkanSemaphore* uploadSemaphore = nullptr;
    if (m_currentBufferState.isCurrentBufferAcitve(CommandTargetType::CmdUploadBuffer))
    {
        VulkanCommandBuffer* uploadBuffer = m_currentBufferState.getAcitveBuffer(CommandTargetType::CmdUploadBuffer);
        if (uploadBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Begin)
        {
            uploadBuffer->endCommandBuffer();
        }

        std::vector<VulkanSemaphore*> uploadSemaphores;
        if (m_currentBufferState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer))
        {
            uploadSemaphore = m_semaphoreManager->acquireSemaphore();
            uploadSemaphores.push_back(uploadSemaphore);
        }

        if (!m_waitSemaphores.empty())
        {
            uploadBuffer->addSemaphores(VK_PIPELINE_STAGE_TRANSFER_BIT, m_waitSemaphores);
            m_waitSemaphores.clear();
        }

        m_cmdBufferManager->submit(uploadBuffer, uploadSemaphores);
        m_currentBufferState.invalidateCommandBuffer(CommandTargetType::CmdUploadBuffer);
    }

    //Draw commands
    if (m_currentBufferState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer))
    {
        VulkanCommandBuffer* drawBuffer = m_currentBufferState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        if (drawBuffer->isInsideRenderPass())
        {
            ASSERT(m_currentContextState->getCurrentTypedPipeline<VulkanGraphicPipeline>(), "nullptr");
            drawBuffer->cmdEndRenderPass();
        }

        drawBuffer->endCommandBuffer();
        m_uniformBufferManager->markToUse(drawBuffer, 0);

        {
            if (uploadSemaphore != nullptr)
            {
                drawBuffer->addSemaphore(VK_PIPELINE_STAGE_TRANSFER_BIT, uploadSemaphore);
            }

            if (drawBuffer->isBackbufferPresented())
            {
                drawBuffer->addSemaphore(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, m_swapchain->getAcquireSemaphore(m_swapchain->currentSwapchainIndex()));
                VulkanSemaphore* semaphore = m_semaphoreManager->acquireSemaphore();
                m_submitSemaphores.push_back(semaphore);
            }

            if (!m_waitSemaphores.empty())
            {
                drawBuffer->addSemaphores(VK_PIPELINE_STAGE_TRANSFER_BIT, m_waitSemaphores);
                m_waitSemaphores.clear();
            }
        }

        m_cmdBufferManager->submit(drawBuffer, m_submitSemaphores);
        if (wait)
        {
            drawBuffer->waitComplete();
        }

        m_currentBufferState.invalidateCommandBuffer(CommandTargetType::CmdDrawBuffer);
    }

    VulkanContext::finalizeCommandBufferSubmit();
    std::swap(m_waitSemaphores, m_submitSemaphores);

#if VULKAN_DUMP
    VulkanDump::getInstance()->flush();
#endif
}

void VulkanContext::clearBackbuffer(const core::Vector4D& color)
{
    ASSERT(m_swapchain, "m_swapchain is nullptr");
    m_swapchain->getBackbuffer()->clear(this, color);
}

void VulkanContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setViewport [%u, %u; %u, %u]", viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
#endif //VULKAN_DEBUG
    if (VulkanContext::isDynamicState(VK_DYNAMIC_STATE_VIEWPORT))
    {
        VkViewport vkViewport = {};
        vkViewport.x = static_cast<f32>(viewport.getLeftX());
        vkViewport.y = static_cast<f32>(viewport.getTopY());
        vkViewport.width = static_cast<f32>(viewport.getWidth());
        vkViewport.height = static_cast<f32>(viewport.getHeight());
        vkViewport.minDepth = depth.x;
        vkViewport.maxDepth = depth.y;
#ifndef PLATFORM_ANDROID
        vkViewport.y = vkViewport.y + vkViewport.height;
        vkViewport.height = -vkViewport.height;
#endif
        std::vector<VkViewport> viewports = { vkViewport };

        VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
        m_currentContextState->setDynamicState(VK_DYNAMIC_STATE_VIEWPORT, std::bind(&VulkanCommandBuffer::cmdSetViewport, drawBuffer, viewports));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanContext::setScissor(const core::Rect32& scissor)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setScissor [%u, %u; %u, %u]", scissor.getLeftX(), scissor.getTopY(), scissor.getWidth(), scissor.getHeight());
#endif //VULKAN_DEBUG
    if (VulkanContext::isDynamicState(VK_DYNAMIC_STATE_SCISSOR))
    {
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.getLeftX(), scissor.getTopY() };
        vkScissor.extent = { static_cast<u32>(scissor.getWidth()), static_cast<u32>(scissor.getHeight()) };
        std::vector<VkRect2D> scissors = { vkScissor };

        VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
        m_currentContextState->setDynamicState(VK_DYNAMIC_STATE_SCISSOR, std::bind(&VulkanCommandBuffer::cmdSetScissor, drawBuffer, scissors));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setRenderTarget");
#endif //VULKAN_DEBUG
    ASSERT(renderpassInfo && framebufferInfo, "nullptr");

    RenderPass* renderpass = m_renderpassManager->acquireRenderPass(renderpassInfo->_desc);
    ASSERT(renderpass, "renderpass is nullptr");
    renderpassInfo->_tracker->attach(renderpass);
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);

    std::vector<VulkanFramebuffer*> vkFramebuffers;
    bool swapchainPresent = std::find(framebufferInfo->_images.cbegin(), framebufferInfo->_images.cend(), nullptr) != framebufferInfo->_images.cend();
    if (swapchainPresent)
    {
        ASSERT(vkRenderpass->isDrawingToSwapchain(), "must be true");
        for (u32 index = 0; index < m_swapchain->getSwapchainImageCount(); ++index)
        {
            std::vector<Image*> images;
            images.reserve(framebufferInfo->_images.size());
            Image* swapchainImage = m_swapchain->getSwapchainImage(index);
            for (auto iter = framebufferInfo->_images.begin(); iter < framebufferInfo->_images.end(); ++iter)
            {
                if (*iter == nullptr)
                {
                    images.push_back(swapchainImage);
                    continue;
                }
                images.push_back(*iter);
            }

            auto [framebuffer, isNewFramebuffer] = m_framebufferManager->acquireFramebuffer(renderpass, images, framebufferInfo->_clearInfo._size);
            ASSERT(framebuffer, "framebuffer is nullptr");

            framebufferInfo->_tracker->attach(framebuffer);
            vkFramebuffers.push_back(static_cast<VulkanFramebuffer*>(framebuffer));
            if (isNewFramebuffer)
            {
                m_swapchain->attachResource(static_cast<VulkanFramebuffer*>(framebuffer), [renderpass](VulkanResource* resource) -> bool
                {
                    VulkanFramebuffer* framebuffer = static_cast<VulkanFramebuffer*>(resource);
                    framebuffer->destroy();
                    if (!framebuffer->create(renderpass))
                    {
                        return false;
                    }

                    return true;
                });
            }
        }
    }
    else
    {
        Framebuffer* framebuffer = nullptr;
        std::tie(framebuffer, std::ignore) = m_framebufferManager->acquireFramebuffer(renderpass, framebufferInfo->_images, framebufferInfo->_clearInfo._size);
        ASSERT(framebuffer, "framebuffer is nullptr");

        framebufferInfo->_tracker->attach(framebuffer);
        vkFramebuffers.push_back(static_cast<VulkanFramebuffer*>(framebuffer));
    }

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setRenderTarget: Renderpass %llx, Framebuffer %llx", vkRenderpass, vkFramebuffers.back());
#endif //VULKAN_DEBUG
    if (!m_currentContextState->isCurrentRenderPass(vkRenderpass) || !m_currentContextState->isCurrentFramebuffer(vkFramebuffers.back()) /*|| clearInfo*/)
    {
        if (m_currentBufferState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer))
        {
            VulkanCommandBuffer* drawBuffer = m_currentBufferState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
            if (drawBuffer->isInsideRenderPass())
            {
                drawBuffer->cmdEndRenderPass();
            }
        }

        //TODO use penging states
        m_currentContextState->setCurrentRenderPass(vkRenderpass);
        m_currentContextState->setCurrentFramebuffer(vkFramebuffers);

        VkRect2D area;
        area.offset = { 0, 0 };
        area.extent = { framebufferInfo->_clearInfo._size.width, framebufferInfo->_clearInfo._size.height };
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setRenderTarget: render area (%d, %d, %d, %d)", area.offset.x, area.offset.y, area.extent.width, area.extent.height);
#endif //VULKAN_DEBUG

        std::vector<VkClearValue> clearValues;
        for (u32 clearIndex = 0; clearIndex < framebufferInfo->_clearInfo._color.size(); ++clearIndex)
        {
            VkClearValue clearColor = {};
            clearColor.color = 
            {{
                framebufferInfo->_clearInfo._color[clearIndex].x,
                framebufferInfo->_clearInfo._color[clearIndex].y,
                framebufferInfo->_clearInfo._color[clearIndex].z,
                framebufferInfo->_clearInfo._color[clearIndex].w 
            }};

            clearValues.push_back(clearColor);
            if (renderpassInfo->_desc._desc._attachments[clearIndex]._autoResolve)
            {
                clearValues.push_back(clearColor);
            }
        }

        if (renderpassInfo->_desc._desc._hasDepthStencilAttahment)
        {
            VkClearValue depthClear = {};
            depthClear.depthStencil.depth = framebufferInfo->_clearInfo._depth;
            depthClear.depthStencil.stencil = framebufferInfo->_clearInfo._stencil;

            clearValues.push_back(depthClear);
            if (VulkanDeviceCaps::getInstance()->supportDepthAutoResolve && renderpassInfo->_desc._desc._attachments.back()._autoResolve)
            {
                clearValues.push_back(depthClear);
            }
        }
        m_currentContextState->setClearValues(area, clearValues);
    }
}

void VulkanContext::removeFramebuffer(Framebuffer* framebuffer)
{
    ASSERT(framebuffer, "nullptr");
    VulkanFramebuffer* vkFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);
    if (m_currentContextState->isCurrentFramebuffer(vkFramebuffer) || vkFramebuffer->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkFramebuffer, [this, vkFramebuffer](VulkanResource* resource) -> void
        {
            m_framebufferManager->removeFramebuffer(vkFramebuffer);
        });
    }
    else
    {
        m_framebufferManager->removeFramebuffer(vkFramebuffer);
    }
}

void VulkanContext::removeRenderPass(RenderPass* renderpass)
{
    ASSERT(renderpass, "nullptr");
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);
    if (m_currentContextState->isCurrentRenderPass(vkRenderpass) || vkRenderpass->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkRenderpass, [this, vkRenderpass](VulkanResource* resource) -> void
        {
            m_renderpassManager->removeRenderPass(vkRenderpass);
        });
    }
    else
    {
        m_renderpassManager->removeRenderPass(vkRenderpass);
    }
}

void VulkanContext::invalidateRenderPass()
{
    ASSERT(m_currentContextState->getCurrentRenderpass(), "nullptr");
    VulkanCommandBuffer* drawBuffer = m_currentBufferState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }
}

void VulkanContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

    Pipeline* pipeline = m_pipelineManager->acquireGraphicPipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
    pipelineInfo->_tracker->attach(pipeline);
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setPipeline %xll", pipeline);
#endif //VULKAN_DEBUG

    m_pendingState.setPendingPipeline(pipeline);
}

void VulkanContext::setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

    Pipeline* pipeline = m_pipelineManager->acquireComputePipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
    pipelineInfo->_tracker->attach(pipeline);
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::setPipeline %xll", pipeline);
#endif //VULKAN_DEBUG

    m_pendingState.setPendingPipeline(pipeline);
}

void VulkanContext::removePipeline(Pipeline* pipeline)
{
    ASSERT(pipeline, "nullptr");
    VulkanResource* vkResource = nullptr;
    if (pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic)
    {
        vkResource = static_cast<VulkanGraphicPipeline*>(pipeline);
    }
    else if (pipeline->getType() == Pipeline::PipelineType::PipelineType_Compute)
    {
        vkResource = static_cast<VulkanComputePipeline*>(pipeline);
    }
    ASSERT(vkResource, "nullptr");
    
    if (m_currentContextState->isCurrentPipeline(pipeline) || vkResource->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkResource, [this, pipeline](VulkanResource* resource) -> void
        {
            m_pipelineManager->removePipeline(pipeline);
        });
    }
    else
    {
        m_pipelineManager->removePipeline(pipeline);
    }
}

Image* VulkanContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::createImage");
    if (target == TextureTarget::TextureCubeMap)
    {
        ASSERT(layers == 6U, "must be 6 layers");
    }
#endif //VULKAN_DEBUG
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };
    VkSampleCountFlagBits vkSamples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(samples);

    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkFormat, vkExtent, vkSamples, layers, flags, name);
}

Image* VulkanContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipLevels, TextureUsageFlags flags, const std::string& name)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::createImage");
    if (target == TextureTarget::TextureCubeMap)
    {
        ASSERT(layers == 6U, "must be 6 layers");
    }
#endif //VULKAN_DEBUG
    VkImageType vkType = VulkanImage::convertTextureTargetToVkImageType(target);
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };

    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkType, vkFormat, vkExtent, layers, mipLevels, VK_IMAGE_TILING_OPTIMAL, flags, name);
}

void VulkanContext::removeImage(Image* image)
{
    ASSERT(image, "nullptr");
    VulkanImage* vkImage = static_cast<VulkanImage*>(image);
    if (vkImage->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkImage, [vkImage](VulkanResource* resource) -> void
        {
            vkImage->notifyObservers();

            vkImage->destroy();
            delete vkImage;
        });
    }
    else
    {
        vkImage->notifyObservers();

        vkImage->destroy();
        delete vkImage;
    }
}

Buffer* VulkanContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, const std::string& name)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::createBuffer");
#endif //VULKAN_DEBUG
    if (type == Buffer::BufferType::BufferType_VertexBuffer || type == Buffer::BufferType::BufferType_IndexBuffer || type == Buffer::BufferType::BufferType_UniformBuffer)
    {
        return new VulkanBuffer(m_bufferMemoryManager, m_deviceInfo._device, type, usageFlag, size, name);
    }

    ASSERT(false, "not supported");
    return nullptr;
}

void VulkanContext::removeBuffer(Buffer* buffer)
{
    ASSERT(buffer, "nullptr");
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    if (vkBuffer->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkBuffer, [vkBuffer](VulkanResource* resource) -> void
        {
            vkBuffer->notifyObservers();

            vkBuffer->destroy();
            delete vkBuffer;
        });
    }
    else
    {
        vkBuffer->notifyObservers();

        vkBuffer->destroy();
        delete vkBuffer;
    }
}

void VulkanContext::removeSampler(Sampler* sampler)
{
    ASSERT(sampler, "nullptr");
    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
    if (vkSampler->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(vkSampler, [this, vkSampler](VulkanResource* resource) -> void
        {
            m_samplerManager->removeSampler(vkSampler);
        });
    }
    else
    {
        m_samplerManager->removeSampler(vkSampler);
    }
}

void VulkanContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::UniformBuffer& bufferData = info._uniformBuffers[bindIndex];
    if (offset == 0)
    {
        ASSERT(bufferData._size * bufferData._array == size, "different size");
    }

    m_currentContextState->updateConstantBuffer(bufferData, offset, size, data);
}

void VulkanContext::bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::StorageImage& storageData = info._storageImages[bindIndex];

    m_currentContextState->bindStorageImage(vkImage, 0, storageData, VulkanImage::makeVulkanImageSubresource(vkImage, layer, mip));
}

void VulkanContext::transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition)
{
    VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }

    const Image* swapchainImage = (VulkanSwapchain::currentSwapchainIndex() == ~0U) ? m_swapchain->getSwapchainImage(0) : m_swapchain->getBackbuffer();

    auto transitionImages(std::move(images));
    for (auto& view : transitionImages)
    {
        if (std::get<0>(view) == nullptr)
        {
            std::get<0>(view) = swapchainImage;
        }
    }

    VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(transition);
    m_currentTransitionState.transitionImages(drawBuffer, transitionImages, newLayout, transition == TransitionOp::TransitionOp_GeneralCompute);
}

void VulkanContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::draw");
#endif //VULKAN_DEBUG
    [[maybe_unused]] bool changed = m_currentContextState->setCurrentVertexBuffers(desc);

    VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (prepareDraw(drawBuffer))
    {
        //if (changed)
        {
            const StreamBufferDescription& desc = m_currentContextState->getStreamBufferDescription();
            if (!desc._vertices.empty())
            {
                drawBuffer->cmdBindVertexBuffers(0, static_cast<u32>(desc._vertices.size()), desc._vertices, desc._offsets);
            }
        }
        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDraw(firstVertex, vertexCount, firstInstance, instanceCount);
    }

    m_currentContextState->invalidateDescriptorSetsState();
}

void VulkanContext::drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::drawIndexed");
#endif //VULKAN_DEBUG
    [[maybe_unused]] bool changed = m_currentContextState->setCurrentVertexBuffers(desc);

    VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (prepareDraw(drawBuffer))
    {
        //if (changed)
        {
            const StreamBufferDescription& descBuff = m_currentContextState->getStreamBufferDescription();
            ASSERT(descBuff._indices, "nullptr");
            VulkanBuffer* indexBuffer = static_cast<VulkanBuffer*>(descBuff._indices);
            drawBuffer->cmdBindIndexBuffers(indexBuffer, descBuff._indicesOffet, (descBuff._indexType == StreamIndexBufferType::IndexType_16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

            ASSERT(!descBuff._vertices.empty(), "empty");
            drawBuffer->cmdBindVertexBuffers(0, static_cast<u32>(descBuff._vertices.size()), descBuff._vertices, descBuff._offsets);
        }
        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDrawIndexed(firstIndex, indexCount, firstInstance, instanceCount, 0);
    }

    m_currentContextState->invalidateDescriptorSetsState();
}

void VulkanContext::dispatchCompute(const core::Dimension3D& groups)
{
    VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer); //TODO compute buffer
    if (prepareDispatch(drawBuffer))
    {
        ASSERT(!drawBuffer->isInsideRenderPass(), "not outside renderpass");
        drawBuffer->cmdDispatch(groups);
    }

    m_currentContextState->invalidateDescriptorSetsState();
}

void VulkanContext::bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::Image& imageData = info._images[bindIndex];

    m_currentContextState->bindTexture(vkImage, 0, imageData, VulkanImage::makeVulkanImageSubresource(vkImage, layer, mip));
}

void VulkanContext::bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
    Sampler* sampler = m_samplerManager->acquireSampler(samplerInfo->_desc);
    ASSERT(sampler, "nullptr");
    samplerInfo->_tracker->attach(sampler);
    const VulkanSampler* vkSampler = static_cast<const VulkanSampler*>(sampler);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::Sampler& samplerData = info._samplers[bindIndex];

    m_currentContextState->bindSampler(vkSampler, samplerData);
}

void VulkanContext::bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer, s32 mip)
{
    ASSERT(image && samplerInfo, "nullptr");
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);

    Sampler* sampler = m_samplerManager->acquireSampler(samplerInfo->_desc);
    ASSERT(sampler, "nullptr");
    samplerInfo->_tracker->attach(sampler);
    const VulkanSampler* vkSampler = static_cast<const VulkanSampler*>(sampler);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::Image& sampledData = info._sampledImages[bindIndex];

    m_currentContextState->bindTexture(vkImage, vkSampler, 0, sampledData, VulkanImage::makeVulkanImageSubresource(vkImage, layer, mip));
}

const DeviceCaps* VulkanContext::getDeviceCaps() const
{
    return &m_deviceCaps;
}

VulkanStagingBufferManager * VulkanContext::getStagingManager()
{
    ASSERT(m_stagingBufferManager, "enable feature");
    return m_stagingBufferManager;
}

const std::vector<VkDynamicState>& VulkanContext::getDynamicStates()
{
    return s_dynamicStates;
}

bool VulkanContext::isDynamicState(VkDynamicState state)
{
    auto iter = std::find(s_dynamicStates.cbegin(), s_dynamicStates.cend(), state);
    if (iter != s_dynamicStates.cend())
    {
        return true;
    }

    return false;
}

Framebuffer* VulkanContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size)
{
    return new VulkanFramebuffer(m_deviceInfo._device, this, images, size);
}

RenderPass* VulkanContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
    return new VulkanRenderPass(m_deviceInfo._device, *renderpassDesc);
}

Pipeline* VulkanContext::createPipeline(Pipeline::PipelineType type)
{
    if (type == Pipeline::PipelineType::PipelineType_Graphic)
    {
        return new VulkanGraphicPipeline(m_deviceInfo._device, this, m_renderpassManager, m_pipelineLayoutManager);
    }
    else if (type == Pipeline::PipelineType::PipelineType_Compute)
    {
        return new VulkanComputePipeline(m_deviceInfo._device, m_pipelineLayoutManager);
    }

    ASSERT(false, "not supported");
    return nullptr;
}

Sampler* VulkanContext::createSampler(const SamplerDescription& desc)
{
    return new VulkanSampler(m_deviceInfo._device, desc);
}

void VulkanContext::invalidateStates()
{
    m_currentContextState->updateDescriptorStates();

    Pipeline* pipeline = m_currentContextState->getCurrentPipeline();
    if (pipeline)
    {
        m_currentContextState->setCurrentPipeline(nullptr);
        m_pendingState.setPendingPipeline(pipeline);
    }
}

bool VulkanContext::prepareDraw(VulkanCommandBuffer* drawBuffer)
{
    ASSERT(drawBuffer, "nullptr");
    ASSERT(m_currentContextState->getCurrentRenderpass(), "not bound");
    if (!drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdBeginRenderpass(m_currentContextState->getCurrentRenderpass(), m_currentContextState->getCurrentFramebuffer(), m_currentContextState->m_renderPassArea, m_currentContextState->m_renderPassClearValues);
    }

    m_currentContextState->invokeDynamicStates();
    if (m_pendingState.isGraphicPipeline())
    {
        if (m_currentContextState->setCurrentPipeline(m_pendingState.takePipeline<VulkanGraphicPipeline>()))
        {
            drawBuffer->cmdBindPipeline(m_currentContextState->getCurrentTypedPipeline<VulkanGraphicPipeline>());
        }
    }
    ASSERT(m_currentContextState->getCurrentTypedPipeline<VulkanGraphicPipeline>() && m_currentContextState->getCurrentTypedPipeline<VulkanGraphicPipeline>()->getType() == Pipeline::PipelineType::PipelineType_Graphic, "not bound");

    std::vector<VkDescriptorSet> sets;
    std::vector<u32> offsets;
    if (m_currentContextState->prepareDescriptorSets<VulkanGraphicPipeline>(drawBuffer, sets, offsets))
    {
        drawBuffer->cmdBindDescriptorSets(m_currentContextState->getCurrentTypedPipeline<VulkanGraphicPipeline>(), 0, static_cast<u32>(sets.size()), sets, offsets);
    }

    return true;
}

bool v3d::renderer::vk::VulkanContext::prepareDispatch(VulkanCommandBuffer* drawBuffer)
{
    ASSERT(drawBuffer, "nullptr");
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }

    if (m_pendingState.isComputePipeline())
    {
        if (m_currentContextState->setCurrentPipeline(m_pendingState.takePipeline<VulkanComputePipeline>()))
        {
            drawBuffer->cmdBindPipeline(m_currentContextState->getCurrentTypedPipeline<VulkanComputePipeline>());
        }
    }
    ASSERT(m_currentContextState->getCurrentTypedPipeline<VulkanComputePipeline>() && m_currentContextState->getCurrentTypedPipeline<VulkanComputePipeline>()->getType() == Pipeline::PipelineType::PipelineType_Compute, "not bound");

    std::vector<VkDescriptorSet> sets;
    [[maybe_unused]] std::vector<u32> offsets;
    if (m_currentContextState->prepareDescriptorSets<VulkanComputePipeline>(drawBuffer, sets, offsets))
    {
        drawBuffer->cmdBindDescriptorSets(m_currentContextState->getCurrentTypedPipeline<VulkanComputePipeline>(), 0, static_cast<u32>(sets.size()), sets);
    }

    return true;
}

void VulkanContext::finalizeCommandBufferSubmit()
{
    m_cmdBufferManager->updateCommandBuffers();
    m_uniformBufferManager->updateUniformBuffers();
    m_semaphoreManager->updateSemaphores();

    m_stagingBufferManager->destroyStagingBuffers();

    invalidateStates();
    m_resourceDeleter.updateResourceDeleter();
}

VulkanCommandBuffer* VulkanContext::getOrCreateAndStartCommandBuffer(CommandTargetType type)
{
    VulkanCommandBuffer* currentBuffer = m_currentBufferState._currentCmdBuffer[type];
    if (!currentBuffer)
    {
        currentBuffer = m_cmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
        m_currentBufferState._currentCmdBuffer[type] = currentBuffer;
    }

    if (currentBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Ready)
    {
        currentBuffer->beginCommandBuffer();
        return currentBuffer;
    }

    if (currentBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Begin)
    {
        return currentBuffer;
    }

    ASSERT(false, "wrong case");
    return nullptr;
}

VulkanSwapchain* VulkanContext::getSwapchain() const
{
    return m_swapchain;
}

void VulkanContext::generateMipmaps(Image* image, u32 layer, TransitionOp state)
{
    if (!image)
    {
        return;
    }

    VulkanCommandBuffer* drawBuffer = m_currentBufferState.acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }

    ASSERT(image, "nullptr");
    VulkanImage* vkImage = static_cast<VulkanImage*>(image);
    vkImage->generateMipmaps(this, layer);

    VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(state);
    drawBuffer->cmdPipelineBarrier(vkImage, VK_PIPELINE_STAGE_TRANSFER_BIT, VulkanTransitionState::selectStageFlagsByImageLayout(newLayout), newLayout);
}

//Another thread
void VulkanContext::handleNotify(const utils::Observable* obj)
{
    const platform::Window* windows = reinterpret_cast<const platform::Window*>(obj);
    if (windows->isValid())
    {
        LOG_WARNING("VulkanContext::notify, native window %llx (from main)", windows->getWindowHandle());
    }
    else
    {
        LOG_WARNING("VulkanContext::notify native window %llx (from main)", windows->getWindowHandle());
    }
}


VulkanContext::CurrentCommandBufferState::CurrentCommandBufferState() noexcept
    : _commandBufferMgr(nullptr)
{
    memset(&_currentCmdBuffer, 0, sizeof(_currentCmdBuffer));
}

VulkanCommandBuffer* VulkanContext::CurrentCommandBufferState::acquireAndStartCommandBuffer(CommandTargetType type)
{
    if (CurrentCommandBufferState::isCurrentBufferAcitve(type))
    {
        return CurrentCommandBufferState::getAcitveBuffer(type);
    }

    ASSERT(_commandBufferMgr, "nullptr");
    _currentCmdBuffer[type] = _commandBufferMgr->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
    if (_currentCmdBuffer[type]->getStatus() != VulkanCommandBuffer::CommandBufferStatus::Ready)
    {
        LOG_ERROR("VulkanContext::CurrentCommandBufferState::acquireAndStartCommandBuffer CommandBufferStatus is Invalid");
    }
    _currentCmdBuffer[type]->beginCommandBuffer();

    return _currentCmdBuffer[type];
}

void VulkanContext::CurrentCommandBufferState::invalidateCommandBuffer(CommandTargetType type)
{
    _currentCmdBuffer[type] = nullptr;
}

VulkanCommandBuffer* VulkanContext::CurrentCommandBufferState::getAcitveBuffer(CommandTargetType type)
{
    VulkanCommandBuffer* currentBuffer = _currentCmdBuffer[type];
    ASSERT(currentBuffer, "nullptr");
    return currentBuffer;
}

bool VulkanContext::CurrentCommandBufferState::isCurrentBufferAcitve(CommandTargetType type) const
{
    return _currentCmdBuffer[type] != nullptr;
}

#if THREADED_PRESENT
PresentThread::PresentThread(VulkanSwapchain* swapchain)
    : m_thread(&PresentThread::presentLoop, this)
    , m_swapchain(swapchain)
    , m_queue(VK_NULL_HANDLE)
    , m_semaphore()
{
    m_index = m_swapchain->acquireImage();
    m_waitSemaphore.notify();
}

PresentThread::~PresentThread()
{
    m_run.store(false, std::memory_order::memory_order_relaxed);
    m_thread.join();
}

void PresentThread::requestAcquireImage(u32& index)
{
    m_waitSemaphore.wait();
    index = m_index;
}

void PresentThread::requestPresent(VkQueue queue, VkSemaphore semaphore)
{
    m_queue = queue;
    m_semaphore = semaphore;

    m_wakeupSemaphore.notify();
}

void PresentThread::internalPresent()
{
    m_waitSemaphore.reset();
    m_swapchain->present(m_queue, {});
}

void PresentThread::internalAcquire()
{
    m_index = m_swapchain->acquireImage();
    m_waitSemaphore.notify();
}

void PresentThread::presentLoop(void* data)
{
    PresentThread* thiz = reinterpret_cast<PresentThread*>(data);
    thiz->m_run.store(true, std::memory_order::memory_order_relaxed);

    while (thiz->m_run.load(std::memory_order::memory_order_relaxed))
    {
        thiz->m_wakeupSemaphore.wait();
        thiz->internalPresent();
        thiz->internalAcquire();
        thiz->m_wakeupSemaphore.reset();
    }
}
#endif //THREADED_PRESENT
} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
