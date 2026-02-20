#include "VulkanDevice.h"

#include "Utils/Logger.h"

#include "Renderer/ShaderProgram.h"
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
#   include "VulkanGraphicPipeline.h"
#   include "VulkanComputePipeline.h"
#   include "VulkanConstantBuffer.h"
#   include "VulkanStagingBuffer.h"
#   include "VulkanDescriptorPool.h"
#   include "VulkanSampler.h"

#ifdef PLATFORM_ANDROID
#   include "Platform/Android/HWCPProfiler.h"
#endif //PLATFORM_ANDROID

namespace v3d
{
namespace renderer
{
namespace vk
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler* g_CPUProfiler = nullptr;
#endif //FRAME_PROFILER_INTERNAL

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
    VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
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

    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_MULTIVIEW_EXTENSION_NAME,
    VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
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
#ifdef VK_EXT_shader_demote_to_helper_invocation
    VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
#endif
#ifdef VK_EXT_calibrated_timestamps
    VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
#endif
};

std::vector<VkDynamicState> VulkanDevice::s_requiredDynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
};

std::vector<VkDynamicState> VulkanDevice::s_supportedDynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
};

bool VulkanDevice::isDynamicStateSupported(VkDynamicState state)
{
    auto iter = std::find(s_supportedDynamicStates.cbegin(), s_supportedDynamicStates.cend(), state);
    if (iter != s_supportedDynamicStates.cend())
    {
        return true;
    }

    return false;
}

const std::vector<VkDynamicState>& VulkanDevice::getDynamicStates()
{
    return s_requiredDynamicStates;
}

VulkanDevice::VulkanDevice(DeviceMaskFlags mask) noexcept
    : m_imageMemoryManager(nullptr)
    , m_bufferMemoryManager(nullptr)

    , m_stagingBufferManager(nullptr)
    , m_semaphoreManager(nullptr)
    , m_framebufferManager(nullptr)
    , m_renderpassManager(nullptr)
    , m_pipelineLayoutManager(nullptr)
    , m_graphicPipelineManager(nullptr)
    , m_computePipelineManager(nullptr)
    , m_samplerManager(nullptr)
    , m_internalCmdBufferManager(nullptr)


#if ENABLE_RENDERDOC_PROFILE
    , m_captureProfiler(V3D_NEW(CaptureRenderdocProfile, memory::MemoryLabel::MemoryRenderCore))
#else
    , m_captureProfiler(nullptr)
#endif
{
    LOG_DEBUG("VulkanDevice created this %llx", this);

    m_renderType = RenderType::Vulkan;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));
    m_deviceInfo._queueMask = (VkQueueFlags)mask;

    m_threadedPools.resize(std::numeric_limits<u16>::digits); //TODO: calculate count threads
    m_maskOfActiveThreadPool = 0b0000000000000000;

    if (m_captureProfiler)
    {
        m_captureProfiler->create();
    }
}

VulkanDevice::~VulkanDevice()
{
    LOG_DEBUG("~VulkanDevice destructor this %llx", this);

    ASSERT(!m_samplerManager, "m_samplerManager is not nullptr");
    ASSERT(!m_computePipelineManager, "m_computePipelineManager is not nullptr");
    ASSERT(!m_graphicPipelineManager, "m_graphicPipelineManager is not nullptr");
    ASSERT(!m_pipelineLayoutManager, "m_pipelineLayoutManager is not nullptr");
    ASSERT(!m_renderpassManager, "m_renderpassManager is not nullptr");
    ASSERT(!m_framebufferManager, "m_framebufferManager not nullptr");
    ASSERT(!m_semaphoreManager, "m_semaphoreManager is not nullptr");
    ASSERT(!m_imageMemoryManager, "m_imageMemoryManager not nullptr");
    ASSERT(!m_bufferMemoryManager, "m_bufferMemoryManager not nullptr");
    ASSERT(!m_stagingBufferManager, "m_stagingBufferManager not nullptr");

    ASSERT(!m_internalCmdBufferManager, "m_internalCmdBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");

    if (m_captureProfiler)
    {
        m_captureProfiler->destroy();
        V3D_DELETE(m_captureProfiler, memory::MemoryLabel::MemoryRenderCore);
    }
}

void VulkanDevice::submit(CmdList* cmd, bool wait)
{
    VulkanDevice::submit(cmd, nullptr, wait);
}

void VulkanDevice::submit(CmdList* cmd, SyncPoint* sync, bool wait)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    g_CPUProfiler->start(0, RenderFrameProfiler::FrameCounter::Submit);
#endif //FRAME_PROFILER_INTERNAL

    ASSERT(cmd, "nullptr");
    VulkanCmdList& cmdList = *static_cast<VulkanCmdList*>(cmd);
    VulkanSyncPoint* syncPoint = static_cast<VulkanSyncPoint*>(sync);

    ASSERT(cmdList.m_concurrencySlot != ~1, "invalid slot. Skip submit?");
    VulkanCommandBufferManager* cmdBufferMgr = m_threadedPools[cmdList.m_concurrencySlot].m_cmdBufferManager;
    ASSERT(cmdBufferMgr, "nullptr");

    //Transition between cmdbuffers
    VulkanCommandBuffer* transitionBuffer = nullptr;
    auto transitionBufferGetter = [this, &transitionBuffer]() -> VulkanCommandBuffer*
        {
            if (!transitionBuffer)
            {
                transitionBuffer = m_internalCmdBufferManager->acquireNewCmdBuffer(Device::DeviceMask::GraphicMask, CommandBufferLevel::PrimaryBuffer);
                transitionBuffer->beginCommandBuffer();
            }

            return transitionBuffer;
        };

    //Resource commands
    VulkanSemaphore* resourceSemaphore = nullptr;
    if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdResourceBuffer)])
    {
        VulkanCommandBuffer* resourceBuffer = cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdResourceBuffer)];

        resourceBuffer->getResourceStateTracker().prepareGlobalState(transitionBufferGetter);
        if (transitionBuffer)
        {
            transitionBuffer->endCommandBuffer();
            m_internalCmdBufferManager->submit(transitionBuffer, {});
            transitionBuffer = nullptr;
        }

        cmdList.m_pendingRenderState.flushBarriers(resourceBuffer);
        resourceBuffer->endCommandBuffer();

        static thread_local std::vector<VulkanSemaphore*> signalResourceSemaphores;
        signalResourceSemaphores.clear();
        if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)])
        {
            resourceSemaphore = m_semaphoreManager->acquireFreeSemaphore();
            signalResourceSemaphores.push_back(resourceSemaphore);
        }

        if (syncPoint && !syncPoint->m_waitSubmitSemaphores.empty())
        {
            resourceBuffer->addSemaphores(VK_PIPELINE_STAGE_TRANSFER_BIT, syncPoint->m_waitSubmitSemaphores);
            syncPoint->m_waitSubmitSemaphores.clear();
        }

        cmdBufferMgr->submit(resourceBuffer, signalResourceSemaphores);
        resourceBuffer->getResourceStateTracker().finalizeGlobalState();
        cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdResourceBuffer)] = nullptr;
    }

    //Draw commands
    static thread_local std::vector<VulkanSemaphore*> signalDrawSemaphores;
    signalDrawSemaphores.clear();
    if (cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)])
    {
        VulkanCommandBuffer* drawBuffer = cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)];
        if (drawBuffer->isInsideRenderPass())
        {
            drawBuffer->cmdEndRenderPass();
        }

        drawBuffer->getResourceStateTracker().prepareGlobalState(transitionBufferGetter);
        if (transitionBuffer)
        {
            transitionBuffer->endCommandBuffer();
            m_internalCmdBufferManager->submit(transitionBuffer, {});
            transitionBuffer = nullptr;
        }

        cmdList.m_pendingRenderState.flushBarriers(drawBuffer);
        cmdList.m_pendingRenderState.flushDebugMarkers(drawBuffer);
        drawBuffer->endCommandBuffer();

        //Sync
        {
            if (resourceSemaphore)
            {
                drawBuffer->addSemaphore(VK_PIPELINE_STAGE_TRANSFER_BIT, resourceSemaphore);
            }

            VulkanSwapchain* swapchain = drawBuffer->getActiveSwapchain();
            if (swapchain)
            {
                swapchain->attachQueueForPresent(VulkanDevice::getQueueByMask(cmdList.getDeviceMask()));
            }

            if (syncPoint)
            {
                if (!syncPoint->m_waitSubmitSemaphores.empty())
                {
                    drawBuffer->addSemaphores(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, syncPoint->m_waitSubmitSemaphores);
                }

                if (!wait)
                {
                    for (auto& sync : cmdList.m_syncPoints)
                    {
                        signalDrawSemaphores.insert(signalDrawSemaphores.end(), std::make_move_iterator(sync->m_signalSubmitSemaphores.begin()), std::make_move_iterator(sync->m_signalSubmitSemaphores.end()));
                    }
                }
            }
        }

        cmdBufferMgr->submit(drawBuffer, signalDrawSemaphores);
        if (wait)
        {
            drawBuffer->waitCompletion();
        }
        drawBuffer->getResourceStateTracker().finalizeGlobalState();
        cmdList.m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)] = nullptr;
    }
#if FRAME_PROFILER_INTERNAL
    g_CPUProfiler->stop(0, RenderFrameProfiler::FrameCounter::Submit);
#endif //FRAME_PROFILER_INTERNAL

    cmdList.postSubmit();

    m_internalCmdBufferManager->updateStatus();
    cmdBufferMgr->updateStatus();
    m_semaphoreManager->updateStatus();

    m_resourceDeleter.resourceGarbageCollect();
}

void VulkanDevice::waitGPUCompletion(CmdList* cmd)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::GPUWait);
#endif //FRAME_PROFILER_INTERNAL
    ASSERT(cmd, "nullptr");
    VulkanCmdList* vkCmdList = static_cast<VulkanCmdList*>(cmd);
    if (vkCmdList->m_concurrencySlot != ~1)
    {
        s32 slot = vkCmdList->m_concurrencySlot;
        m_threadedPools[slot].m_cmdBufferManager->waitQueueCompletion(getQueueByMask(vkCmdList->m_queueMask));
        m_threadedPools[slot].m_cmdBufferManager->updateStatus();
    }
}

CmdList* VulkanDevice::createCommandList_Impl(DeviceMask queueType)
{
    VulkanCmdList* cmdList = V3D_NEW(VulkanCmdList, memory::MemoryLabel::MemoryRenderCore)(this);
    cmdList->m_queueMask = queueType;
    cmdList->m_queueIndex = 0;
    // unknown slot
    cmdList->m_concurrencySlot = ~1;

    m_cmdLists.push_back(cmdList);

    return cmdList;
}

void VulkanDevice::destroyCommandList(CmdList* cmdList)
{
    VulkanCmdList* vkCmdList = static_cast<VulkanCmdList*>(cmdList);
   
    waitGPUCompletion(vkCmdList);

    // TODO
    s32 slot = vkCmdList->m_concurrencySlot;
    m_maskOfActiveThreadPool &= ~(1 << slot);
    m_cmdLists.erase(std::remove(m_cmdLists.begin(), m_cmdLists.end(), vkCmdList));

    V3D_DELETE(vkCmdList, memory::MemoryLabel::MemoryRenderCore);
}

u32 VulkanDevice::prepareConcurrencySlot()
{
    std::lock_guard lock(m_mutex);

    auto getSlotByThread = std::find_if(m_threadedPools.begin(), m_threadedPools.end(), [](const Concurrency& con) -> bool
        {
            return con.m_threadID == std::this_thread::get_id();
        }
    );

    if (getSlotByThread != m_threadedPools.end())
    {
        return std::distance(m_threadedPools.begin(), getSlotByThread);
    }

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

    if (m_deviceCaps._unifiedMemoryManager)
    {
        m_imageMemoryManager = V3D_NEW(SimpleVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(this);
        m_bufferMemoryManager = m_imageMemoryManager;
    }
    else
    {
        m_imageMemoryManager = V3D_NEW(PoolVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(this, m_deviceCaps._memoryImagePoolSize);
        m_bufferMemoryManager = V3D_NEW(PoolVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(this, m_deviceCaps._memoryBufferPoolSize);
    }
    m_stagingBufferManager = V3D_NEW(VulkanStagingBufferManager, memory::MemoryLabel::MemoryRenderCore)(this);

    m_semaphoreManager = V3D_NEW(VulkanSemaphoreManager, memory::MemoryLabel::MemoryRenderCore)(this);

    if (!m_deviceCaps._supportDynamicRendering)
    {
        m_framebufferManager = V3D_NEW(VulkanFramebufferManager, memory::MemoryLabel::MemoryRenderCore)(this);
        m_renderpassManager = V3D_NEW(VulkanRenderpassManager, memory::MemoryLabel::MemoryRenderCore)(this);
    }

    m_pipelineLayoutManager = V3D_NEW(VulkanPipelineLayoutManager, memory::MemoryLabel::MemoryRenderCore)(this);
    m_graphicPipelineManager = V3D_NEW(VulkanGraphicPipelineManager, memory::MemoryLabel::MemoryRenderCore)(this);
    m_computePipelineManager = V3D_NEW(VulkanComputePipelineManager, memory::MemoryLabel::MemoryRenderCore)(this);
    m_samplerManager = V3D_NEW(VulkanSamplerManager, memory::MemoryLabel::MemoryRenderCore)(this);

    m_internalCmdBufferManager = V3D_NEW(VulkanCommandBufferManager, memory::MemoryLabel::MemoryRenderCore)(this, m_semaphoreManager);

#if FRAME_PROFILER_INTERNAL
    g_CPUProfiler = V3D_NEW(RenderFrameProfiler, memory::MemoryLabel::MemoryRenderCore)(static_cast<u32>(m_threadedPools.size()),
        {
            RenderFrameProfiler::FrameCounter::FrameTime,
            RenderFrameProfiler::FrameCounter::SetTarget,
            RenderFrameProfiler::FrameCounter::SetPipeline,
            RenderFrameProfiler::FrameCounter::SetStates,
            RenderFrameProfiler::FrameCounter::BindResources,
            RenderFrameProfiler::FrameCounter::DrawCalls,
            RenderFrameProfiler::FrameCounter::QueryCommands,
            RenderFrameProfiler::FrameCounter::Submit,
            RenderFrameProfiler::FrameCounter::Present,
            RenderFrameProfiler::FrameCounter::UpdateSubmitResorces,
        },
        {
            RenderFrameProfiler::FrameCounter::DrawCalls,
        });
        m_frameProfiler.attach(g_CPUProfiler);
#   if defined(PLATFORM_ANDROID)
        m_frameProfiler.attach(V3D_NEW(android::HWCPProfiler, memory::MemoryLabel::MemoryRenderCore)(
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
#endif //FRAME_PROFILER_INTERNAL
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

    //Extension
    void* vkInstanceExtension = nullptr;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = vkInstanceExtension;
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
    if (VulkanLayers::checkInstanceLayerIsSupported(renderdocLayerName))
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
    void* vkDeviceExtension = nullptr;

    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
    {
        VkPhysicalDeviceTimelineSemaphoreFeatures physicalDeviceTimelineSemaphoreFeatures = {};
        physicalDeviceTimelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        physicalDeviceTimelineSemaphoreFeatures.pNext = vkDeviceExtension;
        physicalDeviceTimelineSemaphoreFeatures.timelineSemaphore = m_deviceCaps._timelineSemaphore;
        vkDeviceExtension = &physicalDeviceTimelineSemaphoreFeatures;
    }

    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
    {
        VkPhysicalDeviceDynamicRenderingFeatures physicalDeviceDynamicRenderingFeatures = {};
        physicalDeviceDynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        physicalDeviceDynamicRenderingFeatures.pNext = vkDeviceExtension;
        physicalDeviceDynamicRenderingFeatures.dynamicRendering = m_deviceCaps._supportDynamicRendering;
        vkDeviceExtension = &physicalDeviceDynamicRenderingFeatures;
    }

    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_MULTIVIEW_EXTENSION_NAME))
    {
        VkPhysicalDeviceMultiviewFeatures physicalDeviceMultiviewFeatures = {};
        physicalDeviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
        physicalDeviceMultiviewFeatures.pNext = vkDeviceExtension;
        physicalDeviceMultiviewFeatures.multiview = m_deviceCaps._supportMultiview;
        physicalDeviceMultiviewFeatures.multiviewGeometryShader = m_deviceCaps._supportMultiview;
        physicalDeviceMultiviewFeatures.multiviewTessellationShader = m_deviceCaps._supportMultiview;
        vkDeviceExtension = &physicalDeviceMultiviewFeatures;
    }

#ifdef VK_EXT_descriptor_indexing
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
    {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT& physicalDeviceDescriptorIndexingFeatures = m_deviceCaps._physicalDeviceDescriptorIndexingFeatures;
        physicalDeviceDescriptorIndexingFeatures.pNext = vkDeviceExtension;
        vkDeviceExtension = &physicalDeviceDescriptorIndexingFeatures;
    }
#endif

#ifdef VK_EXT_custom_border_color
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
    {
        VkPhysicalDeviceCustomBorderColorFeaturesEXT& physicalDeviceCustomBorderColorFeatures = m_deviceCaps._physicalDeviceCustomBorderColorFeatures;
        physicalDeviceCustomBorderColorFeatures.pNext = vkDeviceExtension;
        vkDeviceExtension = &physicalDeviceCustomBorderColorFeatures;
    }
#endif

#ifdef VK_EXT_host_query_reset
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME))
    {
        VkPhysicalDeviceHostQueryResetFeaturesEXT& physicalDeviceHostQueryResetFeatures = m_deviceCaps._physicalDeviceHostQueryResetFeatures;
        physicalDeviceHostQueryResetFeatures.pNext = vkDeviceExtension;
        vkDeviceExtension = &physicalDeviceHostQueryResetFeatures;
    }
#endif

#ifdef VK_EXT_shader_demote_to_helper_invocation
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME))
    {
        VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT physicalDeviceShaderDemoteToHelperInvocationFeatures = m_deviceCaps._physicalDeviceShaderDemoteToHelperInvocationFeatures;
        physicalDeviceShaderDemoteToHelperInvocationFeatures.pNext = vkDeviceExtension;
        vkDeviceExtension = &physicalDeviceShaderDemoteToHelperInvocationFeatures;
    }
#endif

#if VULKAN_DEBUG
    if (VulkanDeviceCaps::checkDeviceExtension(m_deviceInfo._physicalDevice, VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
    {
        VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR physicalDevicePipelineExecutablePropertiesFeatures = {};
        physicalDevicePipelineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
        physicalDevicePipelineExecutablePropertiesFeatures.pNext = vkDeviceExtension;
        physicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo = m_deviceCaps._pipelineExecutablePropertiesEnabled;
        vkDeviceExtension = &physicalDevicePipelineExecutablePropertiesFeatures;
    }
#endif //VULKAN_DEBUG

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = vkDeviceExtension;
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
    if (!m_deviceInfo._device)
    {
        return;
    }

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
            threadPool.m_cmdBufferManager = nullptr;
        }
    }
    m_threadedPools.clear();
    if (m_internalCmdBufferManager)
    {
        m_internalCmdBufferManager->waitCompletion();
        V3D_DELETE(m_internalCmdBufferManager, memory::MemoryLabel::MemoryRenderCore);
        m_internalCmdBufferManager = nullptr;
    }

    m_resourceDeleter.resourceGarbageCollect(true);

    if (m_samplerManager)
    {
        V3D_DELETE(m_samplerManager, memory::MemoryLabel::MemoryRenderCore);
        m_samplerManager = nullptr;
    }

    if (m_computePipelineManager)
    {
        V3D_DELETE(m_computePipelineManager, memory::MemoryLabel::MemoryRenderCore);
        m_computePipelineManager = nullptr;
    }

    if (m_graphicPipelineManager)
    {
        V3D_DELETE(m_graphicPipelineManager, memory::MemoryLabel::MemoryRenderCore);
        m_graphicPipelineManager = nullptr;
    }

    if (m_pipelineLayoutManager)
    {
        m_pipelineLayoutManager->clear();
        V3D_DELETE(m_pipelineLayoutManager, memory::MemoryLabel::MemoryRenderCore);
        m_pipelineLayoutManager = nullptr;
    }

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

    if (m_deviceCaps._unifiedMemoryManager)
    {
        V3D_DELETE(m_imageMemoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_imageMemoryManager = nullptr;
        m_bufferMemoryManager = nullptr;
    }
    else
    {
        V3D_DELETE(m_imageMemoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_imageMemoryManager = nullptr;

        V3D_DELETE(m_bufferMemoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_bufferMemoryManager = nullptr;
    }

    if (m_stagingBufferManager)
    {
        m_stagingBufferManager->destroyStagingBuffers();
        V3D_DELETE(m_stagingBufferManager, memory::MemoryLabel::MemoryRenderCore);
        m_stagingBufferManager = nullptr;
    }

    if (m_semaphoreManager)
    {
        m_semaphoreManager->updateStatus(true);
        m_semaphoreManager->clear();
        V3D_DELETE(m_semaphoreManager, memory::MemoryLabel::MemoryRenderCore);
        m_semaphoreManager = nullptr;
    }

#if DEBUG_OBJECT_MEMORY
    ASSERT(VulkanBuffer::s_objects.empty(), "buffer objects still exist");
    ASSERT(VulkanImage::s_objects.empty(), "image objects still exist");
    ASSERT(VulkanSemaphore::s_objects.empty(), "semaphore objects still exist");
#endif //DEBUG_OBJECT_MEMORY

#if FRAME_PROFILER_INTERNAL
    if (g_CPUProfiler)
    {
        m_frameProfiler.dettach(g_CPUProfiler);
        V3D_DELETE(g_CPUProfiler, memory::MemoryLabel::MemoryRenderCore);
        g_CPUProfiler = nullptr;
    }
#endif //FRAME_PROFILER_INTERNAL

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

Swapchain* VulkanDevice::createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_INTERNAL
    VulkanSwapchain* swapchain = V3D_NEW(VulkanSwapchain, memory::MemoryLabel::MemoryRenderCore)(this, m_semaphoreManager);
    if (!swapchain->create(window, params))
    {
        swapchain->destroy();
        V3D_DELETE(swapchain, memory::MemoryLabel::MemoryRenderCore);

        LOG_FATAL("VulkanDevice::createSwapchain: Cant create VulkanSwapchain");
        return nullptr;
    }
    m_swapchainList.push_back(swapchain);

#if SWAPCHAIN_ON_ADVANCE
    VulkanCommandBuffer* cmdBuffer = m_internalCmdBufferManager->acquireNewCmdBuffer(Device::DeviceMask::GraphicMask, CommandBufferLevel::PrimaryBuffer);
    cmdBuffer->beginCommandBuffer();
    cmdBuffer->cmdPipelineBarrier(swapchain->getCurrentSwapchainImage(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderTexture::makeSubresource(0, 1, 0, 1));
    cmdBuffer->endCommandBuffer();
    std::vector<VulkanSemaphore*> emptySemaphores;
    m_internalCmdBufferManager->submit(cmdBuffer, emptySemaphores);
    m_internalCmdBufferManager->waitCompletion();
#endif

    return swapchain;
}

void VulkanDevice::destroySwapchain(Swapchain* swapchain)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL
    VulkanSwapchain* vkSwapchain = static_cast<VulkanSwapchain*>(swapchain);

    auto found = std::find(m_swapchainList.cbegin(), m_swapchainList.cend(), vkSwapchain);
    ASSERT(found != m_swapchainList.cend(), "not found");
    m_swapchainList.erase(found);

    vkSwapchain->destroy();
    V3D_DELETE(vkSwapchain, memory::MemoryLabel::MemoryRenderCore);
}

SyncPoint* VulkanDevice::createSyncPoint(CmdList* cmd)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_INTERNAL
    ASSERT(cmd, "nullptr");
    VulkanCmdList* vkCmdList = static_cast<VulkanCmdList*>(cmd);

    VulkanSyncPoint* syncPoint = V3D_NEW(VulkanSyncPoint, memory::MemoryLabel::MemoryRenderCore)();
    ASSERT(syncPoint, "nullptr");

    VulkanSemaphore* waitSemaphore = m_semaphoreManager->createSemaphore(VulkanSemaphore::SemaphoreType::Binary, "SubmitWaitSemaphore");
    syncPoint->m_waitSubmitSemaphores.push_back(waitSemaphore);

    VulkanSemaphore* signalSemaphore = m_semaphoreManager->createSemaphore(VulkanSemaphore::SemaphoreType::Binary, "SubmitSignalSemaphore");
    syncPoint->m_signalSubmitSemaphores.push_back(signalSemaphore);

    vkCmdList->m_syncPoints.push_back(syncPoint);

    return syncPoint;
}

void VulkanDevice::destroySyncPoint(CmdList* cmd, SyncPoint* sync)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL
    ASSERT(cmd, "nullptr");
    VulkanCmdList* vkCmdList = static_cast<VulkanCmdList*>(cmd);
    VulkanSyncPoint* vkSync = static_cast<VulkanSyncPoint*>(sync);

    auto found = std::find(vkCmdList->m_syncPoints.cbegin(), vkCmdList->m_syncPoints.cend(), vkSync);
    ASSERT(found != vkCmdList->m_syncPoints.cend(), "not found");
    vkCmdList->m_syncPoints.erase(found);

    for (auto& wait : vkSync->m_waitSubmitSemaphores)
    {
        m_resourceDeleter.addResourceToDelete(wait, [this, wait](VulkanResource* resource) -> void
            {
                m_semaphoreManager->deleteSemaphore(wait);
            });
    }
    vkSync->m_waitSubmitSemaphores.clear();

    for (auto& signal : vkSync->m_signalSubmitSemaphores)
    {
        m_resourceDeleter.addResourceToDelete(signal, [this, signal](VulkanResource* resource) -> void
            {
                m_semaphoreManager->deleteSemaphore(signal);
            });
    }
    vkSync->m_signalSubmitSemaphores.clear();

    V3D_DELETE(vkSync, memory::MemoryLabel::MemoryRenderCore);
}

TextureHandle VulkanDevice::createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, const std::string& name)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::createTexture");
    if (target == TextureTarget::TextureCubeMap)
    {
        ASSERT(layers == 6U, "must be 6 layers");
    }
#endif //VULKAN_DEBUG

    VkImageType vkType = VulkanImage::convertTextureTargetToVkImageType(target);
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension._width, dimension._height, dimension._depth };

    VulkanImage* vkImage = V3D_NEW(VulkanImage, memory::MemoryLabel::MemoryRenderCore)(this, m_imageMemoryManager, vkType, vkFormat, vkExtent, layers, mipmapLevel, VK_IMAGE_TILING_OPTIMAL, flags, name);
    if (!vkImage->create())
    {
        vkImage->destroy();
        V3D_DELETE(vkImage, memory::MemoryLabel::MemoryRenderCore);

        return TextureHandle(nullptr);
    }

    return TextureHandle((RenderTexture*)vkImage);
}

TextureHandle VulkanDevice::createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::createTexture");
    if (target == TextureTarget::TextureCubeMap)
    {
        ASSERT(layers == 6U, "must be 6 layers");
    }
#endif //VULKAN_DEBUG

    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension._width, dimension._height, dimension._depth };
    VkSampleCountFlagBits vkSamples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(samples);

    VulkanImage* vkImage = V3D_NEW(VulkanImage, memory::MemoryLabel::MemoryRenderCore)(this, m_imageMemoryManager, vkFormat, vkExtent, vkSamples, layers, flags, name);
    if (!vkImage->create())
    {
        vkImage->destroy();
        V3D_DELETE(vkImage, memory::MemoryLabel::MemoryRenderCore);

        return TextureHandle(nullptr);
    }

    return TextureHandle((RenderTexture*)vkImage);
}

void VulkanDevice::destroyTexture(TextureHandle texture)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroyTexture %llx", texture);
#endif //VULKAN_DEBUG

    ASSERT(texture.isValid(), "nullptr");
    VulkanImage* vkImage = static_cast<VulkanImage*>(texture.as<RenderTexture>());
    m_resourceDeleter.addResourceToDelete(vkImage, [](VulkanResource* resource) -> void
        {
            VulkanImage* vkImage = static_cast<VulkanImage*>(resource);
            vkImage->destroy();
            V3D_DELETE(vkImage, memory::MemoryLabel::MemoryRenderCore);
        });
}

BufferHandle VulkanDevice::createBuffer(RenderBuffer::Type type, u16 usageFlag, u64 size, const std::string& name)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::createBuffer");
#endif //VULKAN_DEBUG

    VulkanBuffer* vkBuffer = V3D_NEW(VulkanBuffer, memory::MemoryLabel::MemoryRenderCore)(this, m_bufferMemoryManager, type, size, usageFlag, name);
    if (!vkBuffer->create())
    {
        vkBuffer->destroy();
        V3D_DELETE(vkBuffer, memory::MemoryLabel::MemoryRenderCore);

        return BufferHandle(nullptr);
    }

    return BufferHandle((RenderBuffer*)vkBuffer);
}

void VulkanDevice::destroyBuffer(BufferHandle buffer)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroyBuffer %llx", buffer);
#endif //VULKAN_DEBUG

    ASSERT(buffer.isValid(), "nullptr");
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer.as<RenderBuffer>());
    m_resourceDeleter.addResourceToDelete(vkBuffer, [vkBuffer](VulkanResource* resource) -> void
        {
            VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(resource);
            vkBuffer->destroy();
            V3D_DELETE(vkBuffer, memory::MemoryLabel::MemoryRenderCore);
        });
}

void VulkanDevice::destroyFramebuffer(Framebuffer* framebuffer)
{
    if (m_deviceCaps._supportDynamicRendering)
    {
        return;
    }

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroyFramebuffer %llx", framebuffer);
#endif //VULKAN_DEBUG

    ASSERT(framebuffer, "nullptr");
    VulkanFramebuffer* vkFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);
    m_resourceDeleter.addResourceToDelete(vkFramebuffer, [this, vkFramebuffer](VulkanResource* resource) -> void
        {
            m_framebufferManager->removeFramebuffer(vkFramebuffer);
        });
}

void VulkanDevice::destroyRenderpass(RenderPass* renderpass)
{
    if (m_deviceCaps._supportDynamicRendering)
    {
        return;
    }

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroyRenderpass %llx", renderpass);
#endif //VULKAN_DEBUG

    ASSERT(renderpass, "nullptr");
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);
    m_resourceDeleter.addResourceToDelete(vkRenderpass, [this, vkRenderpass](VulkanResource* resource) -> void
        {
            m_renderpassManager->removeRenderPass(vkRenderpass);
        });
}

void VulkanDevice::destroyPipeline(RenderPipeline* pipeline)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroyPipeline %llx", pipeline);
#endif //VULKAN_DEBUG

    ASSERT(pipeline, "nullptr");
    ASSERT(!pipeline->linked(), "must be freed");
    if (pipeline->getType() == RenderPipeline::PipelineType::PipelineType_Graphic)
    {
        VulkanGraphicPipeline* vkPipeline = static_cast<VulkanGraphicPipeline*>(pipeline);
        m_graphicPipelineManager->removePipeline(vkPipeline, m_resourceDeleter);
    }
    else if (pipeline->getType() == RenderPipeline::PipelineType::PipelineType_Compute)
    {
        VulkanComputePipeline* vkPipeline = static_cast<VulkanComputePipeline*>(pipeline);
        m_computePipelineManager->removePipeline(vkPipeline, m_resourceDeleter);
    }
}

void VulkanDevice::destroySampler(Sampler* sampler)
{
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackFuncProfiler(g_CPUProfiler, 0, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_INTERNAL

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanDevice::destroySampler %llx", sampler);
#endif //VULKAN_DEBUG

    ASSERT(sampler, "nullptr");
    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
    m_resourceDeleter.addResourceToDelete(vkSampler, [this, vkSampler](VulkanResource* resource) -> void
        {
            m_samplerManager->removeSampler(vkSampler);
        });
}


VulkanCmdList::VulkanCmdList(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_queueMask(Device::DeviceMask::GraphicMask)
    , m_queueIndex(0)
    , m_threadID(std::this_thread::get_id())
    , m_concurrencySlot(~0U)

    , m_CBOManager(nullptr)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList constructor this %llx", this);
#endif //VULKAN_DEBUG
    memset(m_currentCmdBuffer, 0, sizeof(m_currentCmdBuffer));

    m_CBOManager = V3D_NEW(VulkanConstantBufferManager, memory::MemoryLabel::MemoryRenderCore)(device);
    m_descriptorSetManager = V3D_NEW(VulkanDescriptorSetManager, memory::MemoryLabel::MemoryRenderCore)(device);

    m_pendingRenderState.init(device);
    m_currentRenderState.init(device);
}

VulkanCmdList::~VulkanCmdList()
{
#if VULKAN_DEBUG
    LOG_DEBUG("~VulkanCmdList destructor this %llx", this);
#endif //VULKAN_DEBUG
    if (m_CBOManager)
    {
        V3D_DELETE(m_CBOManager, memory::MemoryLabel::MemoryRenderCore);
        m_CBOManager = nullptr;
    }

    if (m_descriptorSetManager)
    {
        m_descriptorSetManager->updateStatus();

        V3D_DELETE(m_descriptorSetManager, memory::MemoryLabel::MemoryRenderCore);
        m_descriptorSetManager = nullptr;
    }

    ASSERT(!m_CBOManager, "m_CBOManager is not nullptr");
    ASSERT(!m_descriptorSetManager, "m_descriptorSetManager is not nullptr");
    for (auto& cmdBuff : m_currentCmdBuffer)
    {
        ASSERT(!cmdBuff, "must be nullptr");
    }
}

void VulkanCmdList::setViewport(const math::Rect& viewport, const math::float2& depth)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::setViewport [%u, %u; %u, %u]", m_concurrencySlot, viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
    if (VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_VIEWPORT), "must be dynamic");
#endif //VULKAN_DEBUG

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetStates);
#endif //FRAME_PROFILER_INTERNAL

    VkViewport& vkViewport = m_pendingRenderState._viewports;
    vkViewport.x = static_cast<f32>(viewport.getLeftX());
    vkViewport.y = static_cast<f32>(viewport.getTopY());
    vkViewport.width = static_cast<f32>(viewport.getWidth());
    vkViewport.height = static_cast<f32>(viewport.getHeight());
    vkViewport.minDepth = depth._x;
    vkViewport.maxDepth = depth._y;
#ifndef PLATFORM_ANDROID
    vkViewport.y = vkViewport.y + vkViewport.height;
    vkViewport.height = -vkViewport.height;
#endif
    m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_Viewport);
}

void VulkanCmdList::setScissor(const math::Rect& scissor)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::setScissor [%u, %u; %u, %u]", m_concurrencySlot, scissor.getLeftX(), scissor.getTopY(), scissor.getWidth(), scissor.getHeight());
    if (VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_SCISSOR), "must be dynamic");
#endif //VULKAN_DEBUG

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetStates);
#endif //FRAME_PROFILER_INTERNAL

    VkRect2D& vkScissor = m_pendingRenderState._scissors;
    vkScissor.offset.x = scissor.getLeftX();
    vkScissor.offset.y = scissor.getTopY();
    vkScissor.extent.width = static_cast<u32>(scissor.getWidth());
    vkScissor.extent.height = static_cast<u32>(scissor.getHeight());
    m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_Scissors);
}

void VulkanCmdList::setStencilRef(u32 mask)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::setStencilRef [%mask]", m_concurrencySlot, mask);
    if (VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_STENCIL_REFERENCE), "must be dynamic");
#endif //VULKAN_DEBUG

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetStates);
#endif //FRAME_PROFILER_INTERNAL

    m_pendingRenderState._stencilRef = mask;
    m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_StencilRef);
}

void VulkanCmdList::beginRenderTarget(RenderTargetState& rendertarget)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::beginRenderTarget [%s]", m_concurrencySlot, rendertarget.m_name.c_str());
#endif //VULKAN_DEBUG
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetTarget);
#endif //FRAME_PROFILER_INTERNAL
    TRACE_PROFILER_RENDER_SCOPE("beginRenderTarget", color::rgba8::YELLOW);

    const FramebufferDesc& framebufferDesc = rendertarget.getFramebufferDesc();
    const RenderPassDesc& renderpassDesc = rendertarget.getRenderPassDesc();
    if (m_device.getVulkanDeviceCaps()._supportDynamicRendering)
    {
        m_pendingRenderState._framebufferDesc = framebufferDesc;
        m_pendingRenderState._renderpassDesc = renderpassDesc;
    }
    else
    {
        VulkanRenderPass* renderpass = m_device.m_renderpassManager->acquireRenderpass(renderpassDesc, framebufferDesc);
        rendertarget.m_trackerRenderpass.attach(renderpass);

        auto [framebuffer, isNew] = m_device.m_framebufferManager->acquireFramebuffer(renderpass, framebufferDesc);
        rendertarget.m_trackerFramebuffer.attach(framebuffer);

        m_pendingRenderState._renderpass = renderpass;
        m_pendingRenderState._framebuffer = framebuffer;
    }

    if constexpr (sizeof(color::Color) == sizeof(VkClearValue))
    {
        memcpy(m_pendingRenderState._clearValues.data(), framebufferDesc._clearColorValues.data(), sizeof(framebufferDesc._clearColorValues));
        m_pendingRenderState._clearValues[rendertarget.getColorTextureCount()].depthStencil = { framebufferDesc._clearDepthValue, framebufferDesc._clearStencilValue};
    }
    else
    {
        for (u32 i = 0; i < rendertarget.getColorTextureCount(); ++i)
        {
            //TODO cast to float value
            m_pendingRenderState._clearValues[i].color =
            {
                framebufferDesc._clearColorValues[i]._x,
                framebufferDesc._clearColorValues[i]._y,
                framebufferDesc._clearColorValues[i]._z,
                framebufferDesc._clearColorValues[i]._w,
            };
        }
        m_pendingRenderState._clearValues[rendertarget.getColorTextureCount()].depthStencil = { framebufferDesc._clearDepthValue, framebufferDesc._clearStencilValue};
    }

    m_pendingRenderState._insideRenderpass = true;
    m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_RenderPass);
}

void VulkanCmdList::endRenderTarget()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::endRenderTarget", m_concurrencySlot);
#endif //VULKAN_DEBUG

    VulkanCommandBuffer* drawBuffer = m_currentCmdBuffer[toEnumType(CommandTargetType::CmdDrawBuffer)];
    if (drawBuffer && drawBuffer->isInsideRenderPass())
    {
        if (m_device.getVulkanDeviceCaps()._supportDynamicRendering)
        {
            drawBuffer->cmdEndRendering(m_pendingRenderState._renderpassDesc, m_pendingRenderState._framebufferDesc);
        }
        else
        {
            drawBuffer->cmdEndRenderPass();
        }
    }

    m_pendingRenderState._insideRenderpass = false;
    m_pendingRenderState.invalidate();
}

void VulkanCmdList::setPipelineState(GraphicsPipelineState& state)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::setPipelineState [%s]", m_concurrencySlot, state.m_name.c_str());
#endif //VULKAN_DEBUG

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetPipeline);
#endif //FRAME_PROFILER_INTERNAL
    TRACE_PROFILER_RENDER_SCOPE("setPipelineState", color::rgba8::MAGENTA);

    VulkanGraphicPipeline* pipeline = m_device.m_graphicPipelineManager->acquireGraphicPipeline(state);
    state.m_tracker.attach(pipeline);

    if (m_pendingRenderState._graphicPipeline != pipeline)
    {
        m_pendingRenderState._graphicPipeline = pipeline;
        m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_Pipeline);
    }
}

void VulkanCmdList::setPipelineState(ComputePipelineState& state)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::setPipelineState [%s]", m_concurrencySlot, state.m_name.c_str());
#endif //VULKAN_DEBUG

#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::SetPipeline);
#endif //FRAME_PROFILER_INTERNAL
    TRACE_PROFILER_RENDER_SCOPE("setPipelineState", color::rgba8::MAGENTA);

    VulkanComputePipeline* pipeline = m_device.m_computePipelineManager->acquireGraphicPipeline(state);
    state.m_tracker.attach(pipeline);

    if (m_pendingRenderState._computePipeline != pipeline)
    {
        m_pendingRenderState._computePipeline = pipeline;
        m_pendingRenderState.setDirty(DirtyStateMask::DirtyState_Pipeline);
    }
}

void VulkanCmdList::transition(const TextureView& textureView, TransitionOp state)
{
    ASSERT(textureView._texture, "nullptr");
    VulkanImage* image = nullptr;
    if (textureView._texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(textureView._texture->getTextureHandle().as<Swapchain>());
        image = swapchain->getCurrentSwapchainImage();
    }
    else
    {
        image = static_cast<VulkanImage*>(textureView._texture->getTextureHandle().as<RenderTexture>());
    }

    VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(state);
    m_pendingRenderState.addImageBarrier(image, textureView._subresource, newLayout);
}

void VulkanCmdList::bindTexture(const ShaderProgram* program, u32 set, u32 binding, const TextureView& textureView)
{
    TRACE_PROFILER_RENDER_SCOPE("bindTexture", color::rgba8::GREEN);

    ASSERT(textureView._texture, "nullptr");
    VulkanImage* image = nullptr;
    if (textureView._texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(textureView._texture->getTextureHandle().as<Swapchain>());
        image = swapchain->getCurrentSwapchainImage();
    }
    else
    {
        image = static_cast<VulkanImage*>(textureView._texture->getTextureHandle().as<RenderTexture>());
    }

    u32 slot = program->getResourceSlot(set, binding);
    m_pendingRenderState.bind(BindingType::Texture, slot, set, binding, 0, image, textureView._subresource);
}

void VulkanCmdList::bindUAV(const ShaderProgram* program, u32 set, u32 binding, Buffer* buffer)
{
    TRACE_PROFILER_RENDER_SCOPE("bindBuffer", color::rgba8::GREEN);

    ASSERT(buffer, "nullptr");
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer->getBufferHandle().as<RenderBuffer>());
    u32 slot = program->getResourceSlot(set, binding);
    m_pendingRenderState.bind(BindingType::RWBuffer, slot, set, binding, vkBuffer, 0, vkBuffer->getSize());
}

void VulkanCmdList::bindUAV(const ShaderProgram* program, u32 set, u32 binding, Texture* texture)
{
    TRACE_PROFILER_RENDER_SCOPE("bindBuffer", color::rgba8::GREEN);

    ASSERT(texture, "nullptr");
    VulkanImage* vkImage = static_cast<VulkanImage*>(texture->getTextureHandle().as<RenderTexture>());
    u32 slot = program->getResourceSlot(set, binding);
    m_pendingRenderState.bind(BindingType::RWTexture, slot, set, binding, 0, vkImage, RenderTexture::makeSubresource(0, vkImage->getArrayLayers(), 0, 1));
}

void VulkanCmdList::bindSampler(const ShaderProgram* program, u32 set, u32 binding, const SamplerState& sampler)
{
    TRACE_PROFILER_RENDER_SCOPE("bindSampler", color::rgba8::GREEN);

    VulkanSampler* vkSampler = m_device.m_samplerManager->acquireSampler(sampler);
    ASSERT(vkSampler, "nullptr");
    const_cast<SamplerState&>(sampler).m_tracker.attach(vkSampler);

    u32 slot = program->getResourceSlot(set, binding);
    m_pendingRenderState.bind(BindingType::Sampler, slot, set, binding, vkSampler);
}

void VulkanCmdList::bindConstantBuffer(const ShaderProgram* program, u32 set, u32 binding, u32 size, const void* data)
{
    TRACE_PROFILER_RENDER_SCOPE("bindConstantBuffer", color::rgba8::GREEN);

    u32 alignmentSize = math::alignUp<u32>(size, m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().minMemoryMapAlignment);
    ConstantBufferRange range = m_CBOManager->acquireConstantBuffer(alignmentSize);
    [[maybe_unused]] bool updated = VulkanConstantBufferManager::update(range._buffer, range._offset, size, data);

    u32 slot = program->getResourceSlot(set, binding);
    BindingType type = (m_device.getVulkanDeviceCaps()._useDynamicUniforms) ? BindingType::DynamicUniform : BindingType::Uniform;
    m_pendingRenderState.bind(type, slot, set, binding, range._buffer, range._offset, alignmentSize);
}

void VulkanCmdList::bindPushConstant(ShaderType type, u32 size, const void* data)
{
    TRACE_PROFILER_RENDER_SCOPE("bindPushConstant", color::rgba8::GREEN);
    ASSERT(size <= m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxPushConstantsSize, "maxSize");

    m_pendingRenderState.bindPushConstant(type, size, data);
}

void VulkanCmdList::bindDescriptorSet(const ShaderProgram* program, u32 set, const std::vector<Descriptor>& descriptors)
{
    TRACE_PROFILER_RENDER_SCOPE("bindDescriptorSet", color::rgba8::GREEN);

    ASSERT(set < m_device.getVulkanDeviceCaps()._maxDescriptorSets, "set out of range");
    for (const Descriptor& desc : descriptors)
    {
        if (desc._binding == k_invalidBinding)
        {
            m_pendingRenderState.setDirty(DirtyStateMask(DirtyState_DescriptorSet + set));
            continue;
        }

        ASSERT(descriptors.size() <= m_device.getVulkanDeviceCaps()._maxDescriptorBindingsPerSet, "set out of range");
        switch (desc._type)
        {
        case Descriptor::Type::Descriptor_ConstantBuffer:
        {
            ASSERT(desc._resource.index() == Descriptor::Type::Descriptor_ConstantBuffer, "wrong id");
            const Descriptor::ConstantBuffer& CBO = std::get<Descriptor::Type::Descriptor_ConstantBuffer>(desc._resource);
            VulkanCmdList::bindConstantBuffer(program, set, desc._binding, CBO._size, CBO._data);

            break;
        }

        case Descriptor::Type::Descriptor_TextureSampled:
        {
            ASSERT(desc._resource.index() == Descriptor::Type::Descriptor_TextureSampled, "wrong id");
            const TextureView& view = std::get<Descriptor::Type::Descriptor_TextureSampled>(desc._resource);
            VulkanCmdList::bindTexture(program, set, desc._binding, view);

            break;
        }

        case Descriptor::Type::Descriptor_RWTexture:
        {
            ASSERT(desc._resource.index() == Descriptor::Type::Descriptor_RWTexture, "wrong id");
            Texture* texture = std::get<Descriptor::Type::Descriptor_RWTexture>(desc._resource);
            VulkanCmdList::bindUAV(program, set, desc._binding, texture);

            break;
        }

        case Descriptor::Type::Descriptor_RWBuffer:
        {
            ASSERT(desc._resource.index() == Descriptor::Type::Descriptor_RWBuffer, "wrong id");
            Buffer* buffer = std::get<Descriptor::Type::Descriptor_RWBuffer>(desc._resource);
            VulkanCmdList::bindUAV(program, set, desc._binding, buffer);

            break;
        }

        case Descriptor::Type::Descriptor_Sampler:
        {
            ASSERT(desc._resource.index() == Descriptor::Type::Descriptor_Sampler, "wrong id");
            SamplerState* sampler = std::get<Descriptor::Type::Descriptor_Sampler>(desc._resource);
            VulkanCmdList::bindSampler(program, set, desc._binding, *sampler);

            break;
        }

        default:
            ASSERT(false, "unknow type");
        }
    }
}

bool VulkanCmdList::prepareDraw(VulkanCommandBuffer* drawBuffer)
{
    TRACE_PROFILER_RENDER_SCOPE("prepareDraw", color::rgba8::BLACK);
    m_pendingRenderState.flushDebugMarkers(drawBuffer);

    ASSERT(drawBuffer, "nullptr");
    if (!drawBuffer->isInsideRenderPass())
    {
        if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_Barriers))
        {
            m_pendingRenderState.flushBarriers(drawBuffer);
            m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_Barriers);
        }

        if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_RenderPass))
        {
            if (m_device.getVulkanDeviceCaps()._supportDynamicRendering)
            {
                drawBuffer->cmdBeginRendering(m_pendingRenderState._renderpassDesc, m_pendingRenderState._framebufferDesc, m_pendingRenderState._clearValues);
                m_currentRenderState._renderpassDesc = m_pendingRenderState._renderpassDesc;
                m_currentRenderState._framebufferDesc = m_pendingRenderState._framebufferDesc;
            }
            else
            {
                drawBuffer->cmdBeginRenderpass(m_pendingRenderState._renderpass, m_pendingRenderState._framebuffer, m_pendingRenderState._clearValues);
                m_currentRenderState._renderpass = m_pendingRenderState._renderpass;
                m_currentRenderState._framebuffer = m_pendingRenderState._framebuffer;
            }

            m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_RenderPass);
        }
    }

    if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_Barriers))
    {
        VulkanCommandBuffer* transitionBuffer = acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
        m_pendingRenderState.flushBarriers(transitionBuffer, drawBuffer);
        m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_Barriers);
    }

    if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_Viewport))
    {
        drawBuffer->cmdSetViewport(m_pendingRenderState._viewports);
        m_currentRenderState._viewports = m_pendingRenderState._viewports;
        m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_Viewport);
    }

    if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_Scissors))
    {
        drawBuffer->cmdSetScissor(m_pendingRenderState._scissors);
        m_currentRenderState._scissors = m_pendingRenderState._scissors;
        m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_Scissors);
    }

    if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_StencilRef))
    {
        drawBuffer->cmdSetStencilRef(m_pendingRenderState._stencilMask, m_pendingRenderState._stencilRef);
        m_currentRenderState._stencilMask = m_pendingRenderState._stencilMask;
        m_currentRenderState._stencilRef = m_pendingRenderState._stencilRef;
        m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_StencilRef);
    }

    if (m_pendingRenderState.isDirty(DirtyStateMask::DirtyState_Pipeline))
    {
        drawBuffer->cmdBindPipeline(m_pendingRenderState._graphicPipeline);
        m_currentRenderState._graphicPipeline = m_pendingRenderState._graphicPipeline;
        m_pendingRenderState.unsetDirty(DirtyStateMask::DirtyState_Pipeline);
    }
    ASSERT(m_pendingRenderState._graphicPipeline, "must be bound");

    //PushConstant
    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        if (m_pendingRenderState.isDirty(DirtyStateMask(DirtyState_PushConstant + type)))
        {
            drawBuffer->cmdBindPushConstant(m_pendingRenderState._graphicPipeline, getShaderStageFlagsByShaderType(ShaderType(type)), m_pendingRenderState._pushConstant[type]._size, m_pendingRenderState._pushConstant[type]._data);
            m_pendingRenderState.unsetDirty(DirtyStateMask(DirtyState_PushConstant + type));
        }
    }

    //DS
    if (VulkanCmdList::prepareDescriptorSets(drawBuffer))
    {
        drawBuffer->cmdBindDescriptorSets(m_pendingRenderState._graphicPipeline, 0, m_pendingRenderState._descriptorSets, m_pendingRenderState._dynamicOffsets);

        std::swap(m_currentRenderState._descriptorSets, m_pendingRenderState._descriptorSets);
        std::swap(m_currentRenderState._dynamicOffsets, m_pendingRenderState._dynamicOffsets);
        m_pendingRenderState._descriptorSets.clear();
        m_pendingRenderState._dynamicOffsets.clear();
    }

    return true;
}

bool VulkanCmdList::prepareDescriptorSets(VulkanCommandBuffer* drawBuffer)
{
    TRACE_PROFILER_RENDER_SCOPE("prepareDescriptorSets", color::rgba8::BLACK);

    m_pendingRenderState._descriptorSets.clear();
    const VulkanPipelineLayoutDescription& layoutDesc = m_pendingRenderState._graphicPipeline->getPipelineLayoutDescription();
    u32 maxDescriptorSetCount = std::bit_width(layoutDesc._bindingsSetsMask);
    for (u32 indexSet = 0; indexSet < maxDescriptorSetCount; ++indexSet)
    {
        if ((layoutDesc._bindingsSetsMask >> indexSet) == 0 /*|| layoutDesc._bindingsSet[indexSet].empty()*/)
        {
            continue;
        }

        if (m_pendingRenderState.isDirty(DirtyStateMask(DirtyState_DescriptorSet + indexSet)))
        {
            auto& layoutSet = m_pendingRenderState._graphicPipeline->getDescriptorSetLayouts()._setLayouts[indexSet];

            //get free DS and update
            auto&& [pool, set, offset] = m_descriptorSetManager->acquireFreeDescriptorSet(VulkanDescriptorSetLayoutDescription(layoutDesc._bindingsSet[indexSet]), layoutSet);
            m_pendingRenderState._boundSets[indexSet] = set;
            m_pendingRenderState._descriptorSets.push_back(set);

            drawBuffer->captureResource(pool);

            m_descriptorSetManager->updateDescriptorSet(drawBuffer, set, layoutDesc._bindingsSet[indexSet], m_pendingRenderState._boundSetInfo[indexSet]);
            m_pendingRenderState.unsetDirty(DirtyStateMask(DirtyState_DescriptorSet + indexSet));
        }
        else
        {
            ASSERT(m_pendingRenderState._boundSets[indexSet], "set is not bound");
            m_pendingRenderState._descriptorSets.push_back(m_pendingRenderState._boundSets[indexSet]);
        }
    }

    return !m_pendingRenderState._descriptorSets.empty();
}

void VulkanCmdList::draw(const GeometryBufferDesc& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::draw", m_concurrencySlot);
#endif //VULKAN_DEBUG
    TRACE_PROFILER_RENDER_SCOPE("draw", color::rgba8::RED);

    VulkanCommandBuffer* drawBuffer = acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (VulkanCmdList::prepareDraw(drawBuffer)) [[likely]]
    {
        if (desc._vertexBufferCount)
        {
            drawBuffer->cmdBindVertexBuffers(0, desc._vertexBufferCount, desc._vertexBuffers, desc._vertexOffsets, desc._vertexStrides);
        }

        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDraw(firstVertex, vertexCount, firstInstance, instanceCount);
    }
}

void VulkanCmdList::drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 vertexOffest, u32 firstInstance, u32 instanceCount)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext[%u]::drawIndexed", m_concurrencySlot);
#endif //VULKAN_DEBUG
    TRACE_PROFILER_RENDER_SCOPE("drawIndexed", color::rgba8::RED);

    VulkanCommandBuffer* drawBuffer = acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (VulkanCmdList::prepareDraw(drawBuffer)) [[likely]]
    {
        ASSERT(desc._indexBuffer.isValid(), "nullptr");
        VulkanBuffer* indexBuffer = static_cast<VulkanBuffer*>(desc._indexBuffer.as<RenderBuffer>());
        drawBuffer->cmdBindIndexBuffers(indexBuffer, desc._indexOffset, (desc._indexType == IndexBufferType::IndexType_32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);

        ASSERT(!desc._vertexBuffers.empty(), "empty");
        drawBuffer->cmdBindVertexBuffers(0, desc._vertexBufferCount, desc._vertexBuffers, desc._vertexOffsets, desc._vertexStrides);

        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDrawIndexed(firstIndex, indexCount, firstInstance, instanceCount, vertexOffest);
    }
}

void VulkanCmdList::clear(Texture* texture, const color::Color& color)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::clear [%f, %f, %f, %f]", m_concurrencySlot, color[0], color[1], color[2], color[3]);
#endif
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
#endif //FRAME_PROFILER_INTERNAL
    VulkanImage* image = nullptr;
    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    if (texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(texture->getTextureHandle().as<Swapchain>());
        image = swapchain->getCurrentSwapchainImage();
    }
    else
    {
        image = static_cast<VulkanImage*>(texture->getTextureHandle().as<RenderTexture>());
    }

    m_pendingRenderState.flushDebugMarkers(cmdBuffer);

    ASSERT(image, "nullptr");
    image->clear(cmdBuffer, color);
}

void VulkanCmdList::clear(Texture* texture, f32 depth, u32 stencil)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::clear [%f, %u, %u]", m_concurrencySlot, depth, stencil);
#endif
#if FRAME_PROFILER_INTERNAL
    RenderFrameProfiler::StackProfiler stackFrameProfiler(g_CPUProfiler, m_concurrencySlot, RenderFrameProfiler::FrameCounter::FrameTime);
#endif //FRAME_PROFILER_INTERNAL

    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    VulkanImage* image = static_cast<VulkanImage*>(texture->getTextureHandle().as<RenderTexture>());

    m_pendingRenderState.flushDebugMarkers(cmdBuffer);

    ASSERT(image, "nullptr");
    image->clear(cmdBuffer, depth, stencil);
}

void VulkanCmdList::clear(Buffer* buffer, u32 value)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList[%u]::clear [%f, %u]", m_concurrencySlot, value);
#endif

    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdDrawBuffer);
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer->getBufferHandle().as<RenderBuffer>());

    m_pendingRenderState.flushDebugMarkers(cmdBuffer);

    ASSERT(vkBuffer, "nullptr");
    vkBuffer->clear(cmdBuffer, value);
}

bool VulkanCmdList::upload(Texture* texture, u32 size, const void* data)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::uploadData");
#endif

    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
    ASSERT(!texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "swapchain is not supported");
    VulkanImage* image = static_cast<VulkanImage*>(texture->getTextureHandle().as<RenderTexture>());
    bool result = image->upload(cmdBuffer, size, data);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (result && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }

    return result;
}

bool VulkanCmdList::upload(Buffer* buffer, u32 offset, u32 size, const void* data)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::uploadData");
#endif

    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer->getBufferHandle().as<RenderBuffer>());
    bool result = vkBuffer->upload(cmdBuffer, offset, size, data);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (result && !m_pendingRenderState._insideRenderpass  && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }

    return result;
}

void VulkanCmdList::copy(Texture* src, Texture* dst, const math::Dimension3D& size)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCmdList::copy");
#endif

    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
    VulkanImage* vkSrcImage = static_cast<VulkanImage*>(src->getTextureHandle().as<RenderTexture>());
    VulkanImage* vkDstImage = static_cast<VulkanImage*>(dst->getTextureHandle().as<RenderTexture>());

    static auto makeSubresourceLayer = [](VulkanImage* image, u32 mip) -> VkImageSubresourceLayers
        {
            VkImageSubresourceLayers layer = {};
            layer.aspectMask = image->getImageAspectFlags();
            layer.mipLevel = mip;
            layer.baseArrayLayer = 0;
            layer.layerCount = image->getArrayLayers();

            return layer;
        };

    //StackBasedAllocator<VkImageCopy>(src->getMipmapsCount()) alloc;
    static thread_local std::vector<VkImageCopy> regions;
    regions.clear();
    for (u32 mip = 0; mip < src->getMipmapsCount(); ++mip)
    {
        VkImageCopy region = {};
        region.srcOffset = { 0, 0, 0 };
        region.srcSubresource = makeSubresourceLayer(vkSrcImage, mip);
        region.dstOffset = { 0, 0, 0 };
        region.dstSubresource = makeSubresourceLayer(vkDstImage, mip);
        region.extent = { size._width, size._height, size._depth };

        regions.push_back(region);
    }

    static auto getPipelineStage = [](const VulkanImage* vkImage) -> VkPipelineStageFlags
        {
            VkPipelineStageFlags imageStage = 0;
            if (vkImage->hasUsageFlag(TextureUsage::TextureUsage_Sampled))
            {
                imageStage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            if (vkImage->isColorFormat(vkImage->getFormat()) && vkImage->hasUsageFlag(TextureUsage::TextureUsage_Attachment))
            {
                imageStage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }
            else if (vkImage->isDepthStencilFormat(vkImage->getFormat()) && vkImage->hasUsageFlag(TextureUsage::TextureUsage_Attachment))
            {
                imageStage |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }

            if (vkImage->hasUsageFlag(TextureUsage::TextureUsage_Storage))
            {
                imageStage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            return imageStage;
        };

    VkImageLayout srcImageOldLayout = cmdBuffer->getResourceStateTracker().getLayout(vkSrcImage, RenderTexture::Subresource());
    VkPipelineStageFlags srcImageStage = getPipelineStage(vkSrcImage);
    cmdBuffer->cmdPipelineBarrier(vkSrcImage, srcImageStage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkImageLayout dstImageOldLayout = cmdBuffer->getResourceStateTracker().getLayout(vkDstImage, RenderTexture::Subresource());
    VkPipelineStageFlags dstImageStage = getPipelineStage(vkDstImage);
    cmdBuffer->cmdPipelineBarrier(vkDstImage, dstImageStage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    cmdBuffer->cmdCopyImageToImage(vkSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkDstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions);

    static auto getImageLayout = [](const VulkanImage* vkImage, VkImageLayout layout) -> VkImageLayout
        {
            if (layout == VK_IMAGE_LAYOUT_UNDEFINED || layout == VK_IMAGE_LAYOUT_PREINITIALIZED) //first time
            {
                if (vkImage->hasUsageFlag(TextureUsage::TextureUsage_Sampled))
                {
                    layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
            }

            return layout;
        };

    cmdBuffer->cmdPipelineBarrier(vkSrcImage, VK_PIPELINE_STAGE_TRANSFER_BIT, srcImageStage, getImageLayout(vkSrcImage, srcImageOldLayout));
    cmdBuffer->cmdPipelineBarrier(vkDstImage, VK_PIPELINE_STAGE_TRANSFER_BIT, dstImageStage, getImageLayout(vkDstImage, dstImageOldLayout));

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (!m_pendingRenderState._insideRenderpass && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }
}

void VulkanCmdList::copy(const TextureView& src, const TextureView& dst, const math::Dimension3D& size)
{
    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
    VulkanImage* vkSrcImage = static_cast<VulkanImage*>(src._texture->getTextureHandle().as<RenderTexture>());
    VulkanImage* vkDstImage = static_cast<VulkanImage*>(dst._texture->getTextureHandle().as<RenderTexture>());

    static auto getPipelineStage = [](const VulkanImage* vkImage) -> VkPipelineStageFlags
        {
            VkPipelineStageFlags imageStage = 0;
            if (vkImage->hasUsageFlag(TextureUsage::TextureUsage_Sampled))
            {
                imageStage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            if (vkImage->isColorFormat(vkImage->getFormat()) && vkImage->hasUsageFlag(TextureUsage::TextureUsage_Attachment))
            {
                imageStage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }
            else if (vkImage->isDepthStencilFormat(vkImage->getFormat()) && vkImage->hasUsageFlag(TextureUsage::TextureUsage_Attachment))
            {
                imageStage |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }

            if (vkImage->hasUsageFlag(TextureUsage::TextureUsage_Storage))
            {
                imageStage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            return imageStage;
        };

    static auto makeSubresourceLayer = [](VulkanImage* image, u32 mip, u32 baseLayer, u32 countLayers) -> VkImageSubresourceLayers
        {
            VkImageSubresourceLayers layer = {};
            layer.aspectMask = image->getImageAspectFlags();
            layer.mipLevel = mip;
            layer.baseArrayLayer = baseLayer;
            layer.layerCount = countLayers;

            return layer;
        };

    std::vector<VkImageCopy> regions;
    for (u32 mip = src._subresource._baseMip; mip < src._subresource._mips; ++mip)
    {
        VkImageCopy regions = {};
        regions.srcOffset = { 0, 0, 0 };
        regions.srcSubresource = makeSubresourceLayer(vkSrcImage, mip, src._subresource._baseLayer, src._subresource._layers);
        regions.dstOffset = { 0, 0, 0 };
        regions.dstSubresource = makeSubresourceLayer(vkDstImage, mip, dst._subresource._baseLayer, dst._subresource._layers);
        regions.extent = { size._width, size._height, size._depth };
    }

    VkImageLayout srcImageOldLayout = cmdBuffer->getResourceStateTracker().getLayout(vkSrcImage, src._subresource);
    VkPipelineStageFlags srcImageStage = getPipelineStage(vkSrcImage);
    cmdBuffer->cmdPipelineBarrier(vkSrcImage, src._subresource, srcImageStage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkImageLayout dstImageOldLayout = cmdBuffer->getResourceStateTracker().getLayout(vkDstImage, dst._subresource);
    VkPipelineStageFlags dstImageStage = getPipelineStage(vkDstImage);
    cmdBuffer->cmdPipelineBarrier(vkDstImage, dst._subresource, dstImageStage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    cmdBuffer->cmdCopyImageToImage(vkSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkDstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions);

    cmdBuffer->cmdPipelineBarrier(vkSrcImage, src._subresource, VK_PIPELINE_STAGE_TRANSFER_BIT, srcImageStage, srcImageOldLayout);
    cmdBuffer->cmdPipelineBarrier(vkDstImage, dst._subresource, VK_PIPELINE_STAGE_TRANSFER_BIT, dstImageStage, dstImageOldLayout);

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (!m_pendingRenderState._insideRenderpass && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }
}

void VulkanCmdList::copy(Buffer* src, u32 srcOffset, Buffer* dst, u32 dstOffset, u32 size)
{
    VulkanCommandBuffer* cmdBuffer = VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType::CmdResourceBuffer);
    VulkanBuffer* vkSrcBuffer = static_cast<VulkanBuffer*>(src->getBufferHandle().as<RenderBuffer>());
    VulkanBuffer* vkDstBuffer = static_cast<VulkanBuffer*>(dst->getBufferHandle().as<RenderBuffer>());

    VkBufferCopy regions = {};
    regions.srcOffset = static_cast<VkDeviceSize>(srcOffset);
    regions.dstOffset = static_cast<VkDeviceSize>(dstOffset);
    regions.size = static_cast<VkDeviceSize>(size);

    cmdBuffer->cmdCopyBufferToBuffer(vkSrcBuffer, vkDstBuffer, { regions });

    u32 immediateResourceSubmit = m_device.getVulkanDeviceCaps()._immediateResourceSubmit;
    if (!m_pendingRenderState._insideRenderpass && immediateResourceSubmit > 0)
    {
        m_device.submit(this, immediateResourceSubmit == 2 ? true : false);
    }
}

void VulkanCmdList::insertDebugMarker(const std::string& marker, const color::Color& color)
{
    //m_pendingRenderState._debugMarkers.emplace_back(marker, color, false);
}

void VulkanCmdList::beginDebugMarker(const std::string& marker, const color::Color& color)
{
    m_pendingRenderState._debugMarkers.emplace(marker, color, true);
}

void VulkanCmdList::endDebugMarker(const std::string& marker)
{
    m_pendingRenderState._debugMarkers.emplace(marker, color::Color(), false);
}

VulkanCommandBuffer* VulkanCmdList::acquireAndStartCommandBuffer(CommandTargetType type)
{
    VulkanCommandBuffer* cmdBufer = m_currentCmdBuffer[toEnumType(type)];
    if (cmdBufer)
    {
        return cmdBufer;
    }

    // Select slot
    m_concurrencySlot = m_device.prepareConcurrencySlot();
    cmdBufer = m_device.m_threadedPools[m_concurrencySlot].m_cmdBufferManager->acquireNewCmdBuffer(m_queueMask, CommandBufferLevel::PrimaryBuffer);
    cmdBufer->beginCommandBuffer();
    m_currentCmdBuffer[toEnumType(type)] = cmdBufer;

    return cmdBufer;
}

void VulkanCmdList::postSubmit()
{
    m_currentRenderState = std::move(m_pendingRenderState);
    m_pendingRenderState.invalidate();

    m_CBOManager->updateStatus();
    m_descriptorSetManager->updateStatus();
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
