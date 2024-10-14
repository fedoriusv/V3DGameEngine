#include "VulkanDevice.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanImage.h"
#   include "VulkanBuffer.h"
#   include "VulkanSwapchain.h"
#   include "VulkanSemaphore.h"
#   include "VulkanCommandBuffer.h"
#   include "VulkanFramebuffer.h"
#   include "VulkanRenderpass.h"
#   include "VulkanCommandBufferManager.h"

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
#ifdef VK_EXT_host_query_reset
    VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
#endif
};

std::vector<VkDynamicState> VulkanDevice::s_dynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
};

bool VulkanDevice::isDynamicState(VkDynamicState state)
{
    auto iter = std::find(s_dynamicStates.cbegin(), s_dynamicStates.cend(), state);
    if (iter != s_dynamicStates.cend())
    {
        return true;
    }

    return false;
}

const std::vector<VkDynamicState>& VulkanDevice::getDynamicStates()
{
    return s_dynamicStates;
}

VulkanDevice::VulkanDevice(DeviceMaskFlags mask) noexcept
    : m_imageMemoryManager(nullptr)
    , m_bufferMemoryManager(nullptr)

    , m_stagingBufferManager(nullptr)
    , m_semaphoreManager(nullptr)
    , m_framebufferManager(nullptr)
    , m_renderpassManager(nullptr)

{
    LOG_DEBUG("VulkanDevice created this %llx", this);

    m_renderType = RenderType::VulkanRender;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));
    m_deviceInfo._queueMask = (VkQueueFlags)mask;

    m_threadedPools.resize(std::numeric_limits<u16>::digits); //TODO: calculate count threads
    m_maskOfActiveThreadPool = 0b0000000000000000;
}

VulkanDevice::~VulkanDevice()
{
    LOG_DEBUG("~VulkanDevice destructor this %llx", this);

    ASSERT(!m_renderpassManager, "m_renderpassManager is not nullptr");
    ASSERT(!m_framebufferManager, "m_framebufferManager not nullptr");
    ASSERT(!m_semaphoreManager, "m_semaphoreManager is not nullptr");
    ASSERT(!m_imageMemoryManager, "m_imageMemoryManager not nullptr");
    ASSERT(!m_bufferMemoryManager, "m_bufferMemoryManager not nullptr");
    ASSERT(!m_stagingBufferManager, "m_stagingBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");
}

void VulkanDevice::submit(CmdList* cmd, bool wait)
{
    ASSERT(cmd, "nullptr");
    VulkanCmdList& cmdList = *static_cast<VulkanCmdList*>(cmd);

    VulkanCommandBufferManager* cmdBufferMgr = m_threadedPools[cmdList.m_concurrencySlot].m_cmdBufferManager;
    ASSERT(cmdBufferMgr, "nullptr");

    //Uploads commands
    VulkanSemaphore* uploadSemaphore = nullptr;
    if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdUploadBuffer)])
    {
        VulkanCommandBuffer* uploadBuffer = cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdUploadBuffer)];

        cmdList.m_pendingRenderState.flushBarriers(uploadBuffer);
        uploadBuffer->endCommandBuffer();

        static thread_local std::vector<VulkanSemaphore*> uploadSemaphores;
        uploadSemaphores.clear();
        if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)])
        {
            uploadSemaphore = m_semaphoreManager->acquireSemaphore();
            uploadSemaphores.push_back(uploadSemaphore);
        }

        if (!cmdList.m_waitSemaphores.empty())
        {
            uploadBuffer->addSemaphores(VK_PIPELINE_STAGE_TRANSFER_BIT, cmdList.m_waitSemaphores);
            cmdList.m_waitSemaphores.clear();
        }

        cmdBufferMgr->submit(uploadBuffer, uploadSemaphores);
        cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdUploadBuffer)] = nullptr;
    }

    //Draw commands
    if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)])
    {
        VulkanCommandBuffer* drawBuffer = cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)];
        if (drawBuffer->isInsideRenderPass())
        {
            drawBuffer->cmdEndRenderPass();
        }

        cmdList.m_pendingRenderState.flushBarriers(drawBuffer);
        drawBuffer->endCommandBuffer();
        //m_uniformBufferManager->markToUse(drawBuffer, 0);

        {
            if (uploadSemaphore != nullptr)
            {
                drawBuffer->addSemaphore(VK_PIPELINE_STAGE_TRANSFER_BIT, uploadSemaphore);
            }

            if (drawBuffer->isBackbufferPresented())
            {
                drawBuffer->addSemaphores(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cmdList.m_presentSemaphores);
                VulkanSemaphore* semaphore = m_semaphoreManager->acquireSemaphore();
                cmdList.m_submitSemaphores.push_back(semaphore);
            }

            if (!cmdList.m_waitSemaphores.empty())
            {
                drawBuffer->addSemaphores(VK_PIPELINE_STAGE_TRANSFER_BIT, cmdList.m_waitSemaphores);
                cmdList.m_waitSemaphores.clear();
            }
        }

        cmdBufferMgr->submit(drawBuffer, cmdList.m_submitSemaphores);
        if (wait)
        {
            drawBuffer->waitComplete();
        }
        cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)] = nullptr;
    }

    cmdBufferMgr->updateCommandBuffers();
    m_semaphoreManager->updateSemaphores();

    m_resourceDeleter.updateResourceDeleter();

    cmdList.postSubmit();
}

Swapchain* VulkanDevice::createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params)
{
    VulkanSwapchain* swapchain = V3D_NEW(VulkanSwapchain, memory::MemoryLabel::MemoryRenderCore)(this, m_semaphoreManager);
    if (!swapchain->create(window, params))
    {
        swapchain->destroy();
        V3D_FREE(swapchain, memory::MemoryLabel::MemoryRenderCore);

        LOG_FATAL("VulkanDevice::createSwapchain: Cant create VulkanSwapchain");
        return nullptr;
    }

    return swapchain;
}

void VulkanDevice::destroySwapchain(Swapchain* swapchain)
{
    VulkanSwapchain* vkSwapchain = static_cast<VulkanSwapchain*>(swapchain);

    vkSwapchain->destroy();
    V3D_FREE(vkSwapchain, memory::MemoryLabel::MemoryRenderCore);
}

CmdList* VulkanDevice::createCommandList_Impl(DeviceMask queueType)
{
    VulkanCmdList* cmdList = V3D_NEW(VulkanCmdList, memory::MemoryLabel::MemoryRenderCore)(this);
    cmdList->m_queueMask = queueType;
    cmdList->m_queueIndex = 0;
    cmdList->m_concurrencySlot = prepareConcurrencySlot();

    m_cmdLists.push_back(cmdList);

    return cmdList;
}

void VulkanDevice::destroyCommandList(CmdList* cmdList)
{
    VulkanCmdList* vkCmdList = static_cast<VulkanCmdList*>(cmdList);
   
    s32 slot = vkCmdList->m_concurrencySlot;
    m_threadedPools[slot].m_cmdBufferManager->waitQueueCompletion(getQueueByMask(vkCmdList->m_queueMask));
    m_threadedPools[slot].m_cmdBufferManager->updateCommandBuffers();

    m_maskOfActiveThreadPool &= ~(1 << slot);
    V3D_DELETE(vkCmdList, memory::MemoryLabel::MemoryRenderCore);

    m_cmdLists.erase(std::remove(m_cmdLists.begin(), m_cmdLists.end(), vkCmdList));
}

u32 VulkanDevice::prepareConcurrencySlot()
{
    s32 slot = getFreeThreadSlot();
    ASSERT(slot >= 0, ("must be valid"));
    m_maskOfActiveThreadPool |= 1 << slot;

    m_threadedPools[slot].m_cmdBufferManager = V3D_NEW(VulkanCommandBufferManager, memory::MemoryLabel::MemoryRenderCore)(this, m_semaphoreManager);
    m_threadedPools[slot].m_threadID = std::this_thread::get_id();

    return slot;
}

bool VulkanDevice::initialize()
{
    LOG_DEBUG("VulkanDevice::initialize");
    if (!LoadVulkanLibrary())
    {
        LOG_WARNING("VulkanDevice::initialize: LoadVulkanLibrary is falied");
    }

    if (!VulkanDevice::createInstance())
    {
        LOG_FATAL("VulkanDevice::createInstance is failed");

        ASSERT(false, "createInstance is failed");
        return false;
    }

    if (!VulkanDevice::createDevice())
    {
        LOG_FATAL("VulkanDevice::createDevice is failed");
        VulkanDevice::destroy();

        ASSERT(false, "createDevice is failed");
        return false;
    }

    m_deviceCaps.initialize();

    m_semaphoreManager = V3D_NEW(VulkanSemaphoreManager, memory::MemoryLabel::MemoryRenderCore)(this);
    m_framebufferManager = V3D_NEW(VulkanFramebufferManager, memory::MemoryLabel::MemoryRenderCore)(this);
    m_renderpassManager = V3D_NEW(VulkanRenderpassManager, memory::MemoryLabel::MemoryRenderCore)(this);

    return true;
}


bool VulkanDevice::createInstance()
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
            LOG_ERROR("VulkanDevice::createInstance: vkEnumerateInstanceVersion error %s", ErrorString(result).c_str());
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
        LOG_INFO("VulkanDevice::createInstance: extention: [%s]", (*iter).c_str());
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
                LOG_INFO("VulkanDevice::createInstance: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanDevice::createInstance: extention [%s] is not supported", *extentionName);
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
            LOG_INFO("VulkanDevice::createInstance: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

#if VULKAN_RENDERDOC_LAYER
    const c8* renderdocLayerName = "VK_LAYER_RENDERDOC_Capture";
    if (VulkanLayers::checkLayerIsSupported(renderdocLayerName))
    {
        LOG_INFO("VulkanDevice::createInstance: enable layer: [%s]", renderdocLayerName);
        layerNames.push_back(renderdocLayerName);
    }
#endif //VULKAN_RENDERDOC_LAYER

    instanceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

    VkResult result = VulkanWrapper::CreateInstance(&instanceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._instance);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanDevice::createInstance: vkCreateInstance error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._instance))
    {
        LOG_WARNING("VulkanDevice::createInstance: LoadVulkanLibrary is falied");
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
            LOG_ERROR("VulkanDevice::createInstance: createDebugUtilsMessager failed");
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
            LOG_ERROR("VulkanDevice::createInstance: createDebugReportCallback failed");
        }
    }
#endif //VULKAN_LAYERS_CALLBACKS
    return true;
}

bool VulkanDevice::createDevice()
{
    ASSERT(m_deviceInfo._instance != VK_NULL_HANDLE, "instance is nullptr");

    // Physical device
    u32 gpuCount = 0;
    VkResult result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanDevice::createDevice: Error %s", ErrorString(result).c_str());
        return false;
    }
    LOG_INFO("VulkanDevice::createDevice: count GPU: %u", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    result = VulkanWrapper::EnumeratePhysicalDevices(m_deviceInfo._instance, &gpuCount, physicalDevices.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDevice::createDevice: Can not enumerate phyiscal devices. vkEnumeratePhysicalDevices Error %s", ErrorString(result).c_str());
        return false;
    }

    for (u32 index = 0; index < physicalDevices.size(); ++index)
    {
        VkPhysicalDeviceProperties properties = {};
        VulkanWrapper::GetPhysicalDeviceProperties(physicalDevices[index], &properties);

        LOG_INFO("VulkanDevice::createDevice:    [%u]: %s, vendor %s, type %u", index, properties.deviceName, VendorIDString(properties.vendorID).c_str(), properties.deviceType);
    }

    LOG_INFO("VulkanDevice::createDevice: selected first GPU");
    m_deviceInfo._physicalDevice = physicalDevices.front();

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfDeviceExtensions(m_deviceInfo._physicalDevice, supportedExtensions);
#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanDevice::createDevice: extention: [%s]", (*iter).c_str());
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
                LOG_INFO("VulkanDevice::createDevice: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanDevice::createDevice: extention [%s] is not supported", *extentionName);
        }
    }
    VulkanDeviceCaps::s_enableExtensions = enabledExtensions;

    m_deviceCaps.fillCapabilitiesList(&m_deviceInfo);

    ASSERT(m_deviceInfo._queueMask, "is empty");
    std::vector<std::tuple<u32, VkQueueFlags, std::vector<f32>>> queueLists;
    std::array<DeviceMask, 3> masks = { DeviceMask::GraphicMask, DeviceMask::ComputeMask, DeviceMask::TransferMask };
    for (const DeviceMaskFlags flag : masks)
    {
        if (m_deviceInfo._queueMask & flag)
        {
            std::vector<f32> logicalQueuePriority = { 0.0f };
            auto [familyIndex, queueFlags] = m_deviceCaps.getQueueFamiliyIndex(flag);

            queueLists.push_back({ familyIndex, queueFlags, logicalQueuePriority });
        }
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto& requestedQueue : queueLists)
    {
        s32 requestedQueueFamalyIndex = std::get<0>(requestedQueue);
        [[maybe_unused]] VkQueueFlags requestedQueueTypes = std::get<1>(requestedQueue);
        const std::vector<f32>& queuePriority = std::get<2>(requestedQueue);
        s32 requestedQueueCount = static_cast<s32>(std::get<2>(requestedQueue).size());

        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext = nullptr;
        queueInfo.flags = 0;
        queueInfo.queueFamilyIndex = requestedQueueFamalyIndex;
        queueInfo.queueCount = requestedQueueCount;
        queueInfo.pQueuePriorities = queuePriority.data();

        if (std::find_if(queueCreateInfos.cbegin(), queueCreateInfos.cend(), 
            [requestedQueueFamalyIndex](const VkDeviceQueueCreateInfo& info)
            {
                return info.queueFamilyIndex == requestedQueueFamalyIndex;
            }) == queueCreateInfos.cend())
        {
            queueCreateInfos.push_back(queueInfo);
        }
    }

    //Features
    void* vkExtension = nullptr;

#ifdef VK_EXT_descriptor_indexing
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
    {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT& physicalDeviceDescriptorIndexingFeatures = m_deviceCaps._physicalDeviceDescriptorIndexingFeatures;
        physicalDeviceDescriptorIndexingFeatures.pNext = vkExtension;
        vkExtension = &physicalDeviceDescriptorIndexingFeatures;
    }
#endif

#ifdef VK_EXT_custom_border_color
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
    {
        VkPhysicalDeviceCustomBorderColorFeaturesEXT& physicalDeviceCustomBorderColorFeatures = m_deviceCaps._physicalDeviceCustomBorderColorFeatures;
        physicalDeviceCustomBorderColorFeatures.pNext = vkExtension;
        vkExtension = &physicalDeviceCustomBorderColorFeatures;
    }
#endif

#ifdef VK_EXT_host_query_reset
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME))
    {
        VkPhysicalDeviceHostQueryResetFeaturesEXT& physicalDeviceHostQueryResetFeatures = m_deviceCaps._physicalDeviceHostQueryResetFeatures;
        physicalDeviceHostQueryResetFeatures.pNext = vkExtension;
        vkExtension = &physicalDeviceHostQueryResetFeatures;
    }
#endif

#if VULKAN_DEBUG
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
    {
        VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR physicalDevicePipelineExecutablePropertiesFeatures = {};
        physicalDevicePipelineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
        physicalDevicePipelineExecutablePropertiesFeatures.pNext = vkExtension;
        physicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo = m_deviceCaps._pipelineExecutablePropertiesEnabled;
        vkExtension = &physicalDevicePipelineExecutablePropertiesFeatures;
    }
#endif //VULKAN_DEBUG

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = vkExtension;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &m_deviceCaps._deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanLayers::s_validationLayerNames.cbegin(); layerName < VulkanLayers::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanLayers::checkDeviceLayerIsSupported(m_deviceInfo._physicalDevice, *layerName))
        {
            LOG_INFO("VulkanDevice::createDevice: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK
    deviceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    deviceCreateInfo.ppEnabledLayerNames = layerNames.data();

    result = VulkanWrapper::CreateDevice(m_deviceInfo._physicalDevice, &deviceCreateInfo, VULKAN_ALLOCATOR, &m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanDevice::createDevice: vkCreateDevice Error %s", ErrorString(result).c_str());
        return false;
    }

    if (!LoadVulkanLibrary(m_deviceInfo._device))
    {
        LOG_WARNING("VulkanDevice::createDevice: LoadVulkanLibrary is falied");
    }

    m_deviceInfo._queues.resize(queueLists.size(), { VK_NULL_HANDLE, ~0U });
    for (u32 queueIndex = 0; queueIndex < m_deviceInfo._queues.size(); ++queueIndex)
    {
        u32 queueFamalyIndex = std::get<0>(queueLists[queueIndex]);
        VulkanWrapper::GetDeviceQueue(m_deviceInfo._device, queueFamalyIndex, 0, &std::get<0>(m_deviceInfo._queues[queueIndex]));
        std::get<1>(m_deviceInfo._queues[queueIndex]) = queueFamalyIndex;

        ASSERT(std::get<0>(m_deviceInfo._queues[queueIndex]), "must be valid");
    }

    return true;
}

void VulkanDevice::destroy()
{
    //Called from game thread
    LOG_DEBUG("VulkanDevice::destroy");

    VkResult result = VulkanWrapper::DeviceWaitIdle(m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDevice::destroy DeviceWaitIdle is failed. Error: %s", ErrorString(result).c_str());
        ASSERT(false, "error");
    }

    ASSERT(m_cmdLists.empty(), "must be deleted");
    ASSERT(m_swapchainList.empty(), "must be deleted");

    for (auto& threadPool : m_threadedPools)
    {
        if (threadPool.m_cmdBufferManager)
        {
            threadPool.m_cmdBufferManager->waitCompletion();
            V3D_DELETE(threadPool.m_cmdBufferManager, memory::MemoryLabel::MemoryRenderCore);
        }
    }
    m_threadedPools.clear();

    if (m_renderpassManager)
    {
        V3D_DELETE(m_renderpassManager, memory::MemoryLabel::MemoryRenderCore);
        m_renderpassManager = nullptr;
    }

    if (m_framebufferManager)
    {
        V3D_DELETE(m_framebufferManager, memory::MemoryLabel::MemoryRenderCore);
        m_framebufferManager = nullptr;
    }


    //if (m_deviceCaps._unifiedMemoryManager)
    //{
    //    delete m_imageMemoryManager;
    //    m_imageMemoryManager = nullptr;
    //    m_bufferMemoryManager = nullptr;
    //}
    //else
    //{
    //    delete m_imageMemoryManager;
    //    m_imageMemoryManager = nullptr;

    //    delete m_bufferMemoryManager;
    //    m_bufferMemoryManager = nullptr;
    //}

    if (m_semaphoreManager)
    {
        m_semaphoreManager->updateSemaphores();
        m_semaphoreManager->clear();
        V3D_DELETE(m_semaphoreManager, memory::MemoryLabel::MemoryRenderCore);
        m_semaphoreManager = nullptr;
    }

    if (m_deviceInfo._device)
    {
        VulkanWrapper::DestroyDevice(m_deviceInfo._device, VULKAN_ALLOCATOR);
        m_deviceInfo._device = VK_NULL_HANDLE;
        m_deviceInfo._queues.clear();
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



VulkanCmdList::VulkanCmdList(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_queueMask(Device::DeviceMask::GraphicMask)
    , m_queueIndex(0)
    , m_threadID(std::this_thread::get_id())
    , m_concurrencySlot(~0U)
{
    memset(m_currentCmdBuffer, 0, sizeof(m_currentCmdBuffer));
}

VulkanCmdList::~VulkanCmdList()
{
    for (auto& cmdBuff : m_currentCmdBuffer)
    {
        ASSERT(!cmdBuff, "must be nullptr");
    }
}

void VulkanCmdList::setViewport(const math::Rect32& viewport, const math::Vector2D& depth)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::setViewport [%u, %u; %u, %u]", viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
    if (VulkanDevice::isDynamicState(VK_DYNAMIC_STATE_VIEWPORT), "must be dynamic");
#endif //VULKAN_DEBUG

    VkViewport& vkViewport = m_pendingRenderState._viewports;
    vkViewport.x = static_cast<f32>(viewport.getLeftX());
    vkViewport.y = static_cast<f32>(viewport.getTopY());
    vkViewport.width = static_cast<f32>(viewport.getWidth());
    vkViewport.height = static_cast<f32>(viewport.getHeight());
    vkViewport.minDepth = depth.m_x;
    vkViewport.maxDepth = depth.m_y;
#ifndef PLATFORM_ANDROID
    vkViewport.y = vkViewport.y + vkViewport.height;
    vkViewport.height = -vkViewport.height;
#endif

    m_pendingRenderState.setDirty(VulkanRenderState::Viewport);
}

void VulkanCmdList::setScissor(const math::Rect32& scissor)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::setScissor [%u, %u; %u, %u]", scissor.getLeftX(), scissor.getTopY(), scissor.getWidth(), scissor.getHeight());
    if (VulkanDevice::isDynamicState(VK_DYNAMIC_STATE_SCISSOR), "must be dynamic");
#endif //VULKAN_DEBUG

    VkRect2D& vkScissor = m_pendingRenderState._scissors;
    vkScissor.offset.x = scissor.getLeftX();
    vkScissor.offset.y = scissor.getTopY();
    vkScissor.extent.width = static_cast<u32>(scissor.getWidth());
    vkScissor.extent.height = static_cast<u32>(scissor.getHeight());

    m_pendingRenderState.setDirty(VulkanRenderState::Scissors);
}

void VulkanCmdList::setStencilRef(u32 mask)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::setStencilRef [%mask]", mask);
    if (VulkanDevice::isDynamicState(VK_DYNAMIC_STATE_STENCIL_REFERENCE), "must be dynamic");
#endif //VULKAN_DEBUG

    m_pendingRenderState._stencilRef = mask;

    m_pendingRenderState.setDirty(VulkanRenderState::StencilRef);
}

void VulkanCmdList::beginRenderTarget(const RenderTargetState& rendertarget)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::beginRenderTarget [%s]", rendertarget.getName());
#endif //VULKAN_DEBUG

    VulkanRenderPass* renderpass = m_device.m_renderpassManager->acquireRenderpass(rendertarget.getRenderPassDesc(), rendertarget.getName());

    FramebufferDesc targets(rendertarget.getFramebufferDesc());
    for (u32 index = 0; index < rendertarget.getRenderPassDesc()._countColorAttachments; ++index)
    {
        if (rendertarget.getRenderPassDesc()._attachmentsDesc[index]._backbuffer)
        {
            ASSERT(targets._images[index].isValid(), "should be vaild");
            targets._images[index] = TextureHandle(OBJECT_FROM_HANDLE(targets._images[index], VulkanSwapchain)->getCurrentSwapchainImage()); //replace to active swaphain texture
        }
    }

    auto [framebuffer, isNew] = m_device.m_framebufferManager->acquireFramebuffer(renderpass, targets, rendertarget.getName());

    m_pendingRenderState._renderpass = renderpass;
    m_pendingRenderState._framebuffer = framebuffer;
    m_pendingRenderState._insideRenderpass = true;
}

void VulkanCmdList::endRenderTarget()
{
    m_pendingRenderState._insideRenderpass = false;
}

void VulkanCmdList::setPipelineState(GraphicsPipelineState& pipeline)
{
    //m_pendingRenderState._p
}

void VulkanCmdList::transition(const TextureView& view, TransitionOp state)
{
    VulkanImage* image = nullptr;
    if (view._texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        VulkanSwapchain* swapchain = OBJECT_FROM_HANDLE(view._texture->getTextureHandle(), VulkanSwapchain);
        image = swapchain->getCurrentSwapchainImage();
    }
    else
    {
        image = OBJECT_FROM_HANDLE(view._texture->getTextureHandle(), VulkanImage);
    }

    VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(state);
    m_pendingRenderState.addImageBarrier(image, view._subresource, newLayout);
}

void VulkanCmdList::bindTexture(u32 binding, Texture* texture)
{
}

void VulkanCmdList::bindBuffer(u32 binding, Buffer* buffer)
{
}

void VulkanCmdList::bindSampler(u32 binding, SamplerState* sampler)
{
}

void VulkanCmdList::bindConstantBuffer(u32 binding, u32 size, void* data)
{
}

void VulkanCmdList::draw(const GeometryBufferDesc& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
}

void VulkanCmdList::drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
}

void VulkanCmdList::clear(Texture* texture, const render::Color& color)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::clear [%f, %f, %f, %f]", color[0], color[1], color[2], color[3]);
#endif

    VulkanImage* image = nullptr;
    if (texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        VulkanSwapchain* swapchain = OBJECT_FROM_HANDLE(texture->getTextureHandle(), VulkanSwapchain);
        swapchain->attachCmdList(this);

        m_presentSemaphores.push_back(swapchain->getCurrentAcquireSemaphore());
        image = swapchain->getCurrentSwapchainImage();
    }
    else
    {
        image = OBJECT_FROM_HANDLE(texture->getTextureHandle(), VulkanImage);
    }
    ASSERT(image, "nullptr");
    image->clear(this, color);
}

void VulkanCmdList::clear(Texture* texture, f32 depth, u32 stencil)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::clear [%f, %u]", depth, stencil);
#endif

    VulkanImage* image = OBJECT_FROM_HANDLE(texture->getTextureHandle(), VulkanImage);
    ASSERT(image, "nullptr");
    image->clear(this, depth, stencil);
}

bool VulkanCmdList::uploadData(Texture2D* texture, const math::Dimension2D& offset, const math::Dimension2D& size, u32 mipLevel, const void* data)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::uploadData");
#endif

    ASSERT(texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "swapchain is not supported");
    VulkanImage* image = OBJECT_FROM_HANDLE(texture->getTextureHandle(), VulkanImage);
    bool result = image->upload(this, math::Dimension3D(offset.m_width, offset.m_height, 0), math::Dimension3D(size.m_width, size.m_height, 0), mipLevel, data);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (result && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }

    return result;
}

bool VulkanCmdList::uploadData(Texture3D* texture, const math::Dimension3D& offset, const math::Dimension3D& size, u32 mipLevel, const void* data)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::uploadData");
#endif

    ASSERT(texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "swapchain is not supported");
    VulkanImage* image = OBJECT_FROM_HANDLE(texture->getTextureHandle(), VulkanImage);
    bool result = image->upload(this, offset, size, mipLevel, data);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (result && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }

    return result;
}

bool VulkanCmdList::uploadData(Buffer* buffer, u32 offset, u32 size, void* data)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::uploadData");
#endif

    VulkanBuffer* vkBuffer = OBJECT_FROM_HANDLE(buffer->getBufferHandle(), VulkanBuffer);
    bool result = vkBuffer->upload(this, offset, size, data);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (result && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }

    return result;
}

VulkanCommandBuffer* VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType type)
{
    if (m_currentCmdBuffer[toEnumType(type)])
    {
        return m_currentCmdBuffer[toEnumType(type)];
    }

    m_currentCmdBuffer[toEnumType(type)] = m_device.m_threadedPools[m_concurrencySlot].m_cmdBufferManager->acquireNewCmdBuffer(m_queueMask, CommandBufferLevel::PrimaryBuffer);
    m_currentCmdBuffer[toEnumType(type)]->beginCommandBuffer();

    return m_currentCmdBuffer[toEnumType(type)];
}

void VulkanCmdList::postSubmit()
{
    m_waitSemaphores = std::move(m_submitSemaphores);
    m_submitSemaphores.clear();

    m_currentRenderState = std::move(m_pendingRenderState);
    m_pendingRenderState.invalidate();
}

void VulkanCmdList::postPresent()
{
    m_presentSemaphores.clear();
    m_waitSemaphores.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
