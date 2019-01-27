#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanStagingBuffer.h"

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


std::vector<VkDynamicState> VulkanGraphicContext::s_dynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    //VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    VK_DYNAMIC_STATE_STENCIL_REFERENCE
};

VulkanGraphicContext::VulkanGraphicContext(const platform::Window * window) noexcept
    : m_deviceCaps(*VulkanDeviceCaps::getInstance())
    , m_swapchain(nullptr)
    , m_cmdBufferManager(nullptr)
    , m_stagingBufferManager(nullptr)

    , m_imageMemoryManager(nullptr)
    , m_bufferMemoryManager(nullptr)

    , m_renderpassManager(nullptr)
    , m_framebuferManager(nullptr)
    , m_pipelineManager(nullptr)

    , m_window(window)
{
    LOG_DEBUG("VulkanGraphicContext created this %llx", this);

    m_renderType = RenderType::VulkanRender;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));
}

VulkanGraphicContext::~VulkanGraphicContext()
 {
    LOG_DEBUG("~VulkanGraphicContext destructor this %llx", this);

    ASSERT(!m_imageMemoryManager, "m_imageMemoryManager not nullptr");
    ASSERT(!m_bufferMemoryManager, "m_bufferMemoryManager not nullptr");
    ASSERT(!m_stagingBufferManager, "m_stagingBufferManager not nullptr");


    ASSERT(!m_renderpassManager, "m_renderpassManager not nullptr");
    ASSERT(!m_framebuferManager, "m_framebuferManager not nullptr");
    ASSERT(!m_pipelineManager, "m_pipelineManager not nullptr");

    ASSERT(!m_swapchain, "m_swapchain not nullptr");
    ASSERT(!m_cmdBufferManager, "m_cmdBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");
 }

void VulkanGraphicContext::beginFrame()
{
    u32 index = m_swapchain->acquireImage();
    LOG_DEBUG("VulkanGraphicContext::beginFrame %llu, image index %u", m_frameCounter, index);

    ASSERT(!m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "buffer exist");
    VulkanCommandBuffer* drawBuffer = m_currentContextState._currentCmdBuffer[CommandTargetType::CmdDrawBuffer] = m_cmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);

    if (drawBuffer->getStatus() != VulkanCommandBuffer::CommandBufferStatus::Ready)
    {
        LOG_ERROR("VulkanGraphicContext::beginFrame CommandBufferStatus is Invalid");
    }
    drawBuffer->beginCommandBuffer();
    drawBuffer->cmdPipelineBarrier(m_swapchain->getBackbuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanGraphicContext::endFrame()
{
    LOG_DEBUG("VulkanGraphicContext::endFrame %llu", m_frameCounter);
    if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdUploadBuffer))
    {
        VulkanCommandBuffer* uploadBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdUploadBuffer);
        m_cmdBufferManager->submit(uploadBuffer, VK_NULL_HANDLE);
        uploadBuffer->waitComplete();
        m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdUploadBuffer);
    }

    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "m_currentDrawBuffer is nullptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        ASSERT(m_currentContextState._currentRenderpass, "nullptr");
        drawBuffer->cmdEndRenderPass();
    }
    drawBuffer->cmdPipelineBarrier(m_swapchain->getBackbuffer(),VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    drawBuffer->endCommandBuffer();
    m_cmdBufferManager->submit(drawBuffer, VK_NULL_HANDLE);
    m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdDrawBuffer);
}

void VulkanGraphicContext::presentFrame()
{
    LOG_DEBUG("VulkanGraphicContext::presentFrame %llu", m_frameCounter);

    std::vector<VkSemaphore> semaphores;
    m_swapchain->present(m_queueList[0], semaphores);

    m_cmdBufferManager->updateCommandBuffers();
    if (m_deviceCaps.useStagingBuffers)
    {
        m_stagingBufferManager->destroyStagingBuffers();
    }
    m_frameCounter++;
}

void VulkanGraphicContext::submit(bool wait)
{
    if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdUploadBuffer))
    {
        VulkanCommandBuffer* uploadBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdUploadBuffer);
        m_cmdBufferManager->submit(uploadBuffer, VK_NULL_HANDLE);
        uploadBuffer->waitComplete();
        m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdUploadBuffer);
    }

    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Begin)
    {
        drawBuffer->endCommandBuffer();
        m_cmdBufferManager->submit(drawBuffer, VK_NULL_HANDLE);
        if (wait)
        {
            drawBuffer->waitComplete();
        }
        m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdDrawBuffer);
    }
}

void VulkanGraphicContext::clearBackbuffer(const core::Vector4D & color)
{
    ASSERT(m_swapchain, "m_swapchain is nullptr");
    m_swapchain->getBackbuffer()->clear(this, color);
}

void VulkanGraphicContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
    LOG_DEBUG("VulkanGraphicContext::setViewport [%u, %u; %u, %u]", viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
    if (VulkanGraphicContext::isDynamicState(VK_DYNAMIC_STATE_VIEWPORT))
    {
        VkViewport vkViewport = {};
        vkViewport.x = static_cast<f32>(viewport.getLeftX());
        vkViewport.y = static_cast<f32>(viewport.getTopY());
        vkViewport.width = static_cast<f32>(viewport.getWidth());
        vkViewport.height = static_cast<f32>(viewport.getHeight());
        vkViewport.minDepth = depth.x;
        vkViewport.maxDepth = depth.y;
        std::vector<VkViewport> viewports = { vkViewport };
 
        VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        m_currentContextState.setDynamicState(VK_DYNAMIC_STATE_VIEWPORT, std::bind(&VulkanCommandBuffer::cmdSetViewport, drawBuffer, viewports));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanGraphicContext::setScissor(const core::Rect32 & scissor)
{
    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
    LOG_DEBUG("VulkanGraphicContext::setScissor [%u, %u; %u, %u]", scissor.getLeftX(), scissor.getTopY(), scissor.getWidth(), scissor.getHeight());
    if (VulkanGraphicContext::isDynamicState(VK_DYNAMIC_STATE_SCISSOR))
    {
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.getLeftX(), scissor.getTopY() };
        vkScissor.offset = { scissor.getWidth(), scissor.getHeight() };

        std::vector<VkRect2D> scissors = { vkScissor };

        VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        m_currentContextState.setDynamicState(VK_DYNAMIC_STATE_SCISSOR, std::bind(&VulkanCommandBuffer::cmdSetScissor, drawBuffer, scissors));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo, 
    const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers)
{
    RenderPass* renderpass = m_renderpassManager->acquireRenderPass(*renderpassInfo);
    ASSERT(renderpass, "renderpass is nullptr");
    std::get<0>(trackers)->attach(renderpass);

    Framebuffer* framebuffer = m_framebuferManager->acquireFramebuffer(renderpass, attachments, clearInfo->_size);
    ASSERT(framebuffer, "framebuffer is nullptr");
    std::get<1>(trackers)->attach(framebuffer);

    if (!m_currentContextState.isCurrentRenderPass(renderpass) || !m_currentContextState.isCurrentFramebuffer(framebuffer) /*|| clearInfo*/)
    {
        m_currentContextState._currentRenderpass = static_cast<VulkanRenderPass*>(renderpass);
        m_currentContextState._currentFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

        VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        if (drawBuffer->isInsideRenderPass())
        {
            drawBuffer->cmdEndRenderPass();
        }

        VkRect2D area;
        area.offset = { 0, 0 };
        area.extent = { clearInfo->_size.width, clearInfo->_size.height };

        std::vector<VkClearValue> clearValues(clearInfo->_color.size() + (renderpassInfo->_hasDepthStencilAttahment) ? 1 : 0);
        for (u32 index = 0; index < clearInfo->_color.size(); ++index)
        {
            clearValues[index] = { clearInfo->_color[index].x, clearInfo->_color[index].y, clearInfo->_color[index].z, clearInfo->_color[index].w };
        }

        if (renderpassInfo->_hasDepthStencilAttahment)
        {
            clearValues.back().depthStencil = { clearInfo->_depth, clearInfo->_stencil };
        }

        drawBuffer->cmdBeginRenderpass(m_currentContextState._currentRenderpass, m_currentContextState._currentFramebuffer, area, clearValues);
    }
}

/*void VulkanGraphicContext::removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo)
{
    ASSERT(false, "need implement correct");
    m_framebuferManager->removeFramebuffer(attachments);
    m_renderpassManager->removeRenderPass(*renderpassInfo);
}*/

void VulkanGraphicContext::removeFramebuffer(Framebuffer * framebuffer)
{
    VulkanFramebuffer* vkFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);
    if (m_currentContextState.isCurrentFramebuffer(vkFramebuffer) || vkFramebuffer->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }

    m_framebuferManager->removeFramebuffer(framebuffer);
}

void VulkanGraphicContext::removeRenderPass(RenderPass * renderpass)
{
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);
    if (m_currentContextState.isCurrentRenderPass(vkRenderpass) || vkRenderpass->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }

    m_renderpassManager->removeRenderPass(renderpass);
}

void VulkanGraphicContext::invalidateRenderPass()
{
    ASSERT(m_currentContextState._currentRenderpass, "nuulptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }
}

void VulkanGraphicContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo, ObjectTracker<Pipeline>* tracker)
{
    ASSERT(pipelineInfo, "nullptr");

    Pipeline* pipeline = m_pipelineManager->acquireGraphicPipeline(pipelineInfo);
    ASSERT(pipeline, "nullptr");
    tracker->attach(pipeline);

    if (!m_currentContextState.isCurrentPipeline(pipeline))
    {
        m_currentContextState._currentPipeline = static_cast<VulkanGraphicPipeline*>(pipeline);
        m_currentContextState._updatePipeline = true;
    }
}

void VulkanGraphicContext::removePipeline(Pipeline * pipeline)
{
    VulkanGraphicPipeline* vkPipeline = static_cast<VulkanGraphicPipeline*>(pipeline);;
    if (m_currentContextState.isCurrentPipeline(vkPipeline) || vkPipeline->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }

    m_pipelineManager->removePipeline(pipeline);
}

Image * VulkanGraphicContext::createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipLevels,
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    VkImageType vkType = VulkanImage::convertTextureTargetToVkImageType(target);
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };

    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkType, vkFormat, vkExtent, mipLevels, VK_IMAGE_TILING_OPTIMAL);
}

Image * VulkanGraphicContext::createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, 
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };
    VkSampleCountFlagBits vkSamples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(samples);

    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkFormat, vkExtent, vkSamples);
}

void VulkanGraphicContext::removeImage(Image * image)
{
    VulkanImage* vkImage = static_cast<VulkanImage*>(image);
    if (vkImage->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }
    else
    {
        vkImage->notifyObservers();

        vkImage->destroy();
        delete vkImage;
    }
}

Buffer * VulkanGraphicContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size)
{
    if (type == Buffer::BufferType::BufferType_VertexBuffer || type == Buffer::BufferType::BufferType_IndexBuffer || type == Buffer::BufferType::BufferType_UniformBuffer)
    {
        return new VulkanBuffer(m_bufferMemoryManager, m_deviceInfo._device, type, usageFlag, size);
    }

    ASSERT(false, "not supported");
    return nullptr;

}

void VulkanGraphicContext::removeBuffer(Buffer * buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    if (vkBuffer->isCaptured())
    {
        //delayed destoy
        ASSERT(false, "not implementing");
    }
    else
    {
        buffer->notifyObservers();

        buffer->destroy();
        delete buffer;
    }
}

void VulkanGraphicContext::bindTexture(const resource::Shader* shader, const std::string& name, const Image* image)
{
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);
    VkImageView view = vkImage->getImageView();

    //shader->getReflectionInfo()._sampledImages[name]
}

void VulkanGraphicContext::bindUniformBuffers(const resource::Shader* shader, const std::string& name, const void* data, u32 offset, u32 size)
{
    //m_currentContextState._boundUniformBuffers = { {buffer}, {}, true };
}

void VulkanGraphicContext::bindVertexBuffers(const std::vector<Buffer*>& buffer, const std::vector<u64>& offsets)
{
    m_currentContextState._boundVertexBuffers = { buffer, offsets, true };
}

void VulkanGraphicContext::draw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
    if (prepareDraw())
    {
        VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        if (std::get<2>(m_currentContextState._boundVertexBuffers))
        {
            drawBuffer->cmdBindVertexBuffers(0, static_cast<u32>(std::get<0>(m_currentContextState._boundVertexBuffers).size()), std::get<0>(m_currentContextState._boundVertexBuffers), std::get<1>(m_currentContextState._boundVertexBuffers));
            std::get<2>(m_currentContextState._boundVertexBuffers) = false;
        }
        drawBuffer->cmdDraw(firstVertex, vertexCount, firstInstance, instanceCount);
    }
}

void VulkanGraphicContext::drawIndexed()
{
    //TODO:
}

const DeviceCaps* VulkanGraphicContext::getDeviceCaps() const
{
    return &m_deviceCaps;
}

VulkanStaginBufferManager * VulkanGraphicContext::getStagingManager()
{
    ASSERT(m_deviceCaps.useStagingBuffers, "enable feature");
    return m_stagingBufferManager;
}

const std::vector<VkDynamicState>& VulkanGraphicContext::getDynamicStates()
{
    return s_dynamicStates;
}

bool VulkanGraphicContext::isDynamicState(VkDynamicState state)
{
    auto iter = std::find(s_dynamicStates.cbegin(), s_dynamicStates.cend(), state);
    if (iter != s_dynamicStates.cend())
    {
        return true;
    }

    return false;
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

    m_deviceCaps.initialize();

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

    if (m_deviceCaps.unifiedMemoryManager)
    {
        m_imageMemoryManager = new VulkanMemory(m_deviceInfo._device);
        m_bufferMemoryManager = m_imageMemoryManager;
    }
    else
    {
        m_imageMemoryManager = new VulkanMemory(m_deviceInfo._device);
        m_bufferMemoryManager = new VulkanMemory(m_deviceInfo._device);
    }

    m_cmdBufferManager = new VulkanCommandBufferManager(&m_deviceInfo, m_queueList[0]);
    //m_currentContextState._currentDrawBuffer = m_drawCmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
    //ASSERT(m_currentContextState._currentDrawBuffer, "m_currentDrawBuffer is nullptr");

    if (m_deviceCaps.useStagingBuffers)
    {
        m_stagingBufferManager = new VulkanStaginBufferManager(m_deviceInfo._device);
    }
    m_renderpassManager = new RenderPassManager(this);
    m_framebuferManager = new FramebufferManager(this);
    m_pipelineManager = new PipelineManager(this);

    return true;
}

void VulkanGraphicContext::destroy()
{
    //Called from game thread
    LOG_DEBUG("VulkanGraphicContext::destroy");

    //TODO: Wait to complete all commands
    if (m_cmdBufferManager)
    {
        m_cmdBufferManager->waitCompete();

        delete m_cmdBufferManager;
        m_cmdBufferManager = nullptr;
    }

    if (m_stagingBufferManager)
    {
        delete m_stagingBufferManager;
        m_stagingBufferManager = nullptr;
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

    if (m_renderpassManager)
    {
        delete m_renderpassManager;
        m_renderpassManager = nullptr;
    }

    if (m_framebuferManager)
    {
        delete m_framebuferManager;
        m_framebuferManager = nullptr;
    }

    if (m_pipelineManager)
    {
        delete m_pipelineManager;
        m_pipelineManager = nullptr;
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

Framebuffer * VulkanGraphicContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D & size)
{
    return new VulkanFramebuffer(m_deviceInfo._device, images, size);
}

RenderPass * VulkanGraphicContext::createRenderPass(const RenderPass::RenderPassInfo * renderpassInfo)
{
    u32 countAttachments = (renderpassInfo->_hasDepthStencilAttahment) ? renderpassInfo->_countColorAttachments + 1 : renderpassInfo->_countColorAttachments;
    std::vector<VulkanRenderPass::VulkanAttachmentDescription> descs(countAttachments);
    for (u32 index = 0; index < renderpassInfo->_countColorAttachments; ++index)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = descs[index];
        desc._format = VulkanImage::convertImageFormatToVkFormat(renderpassInfo->_attachments[index]._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(renderpassInfo->_attachments[index]._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(renderpassInfo->_attachments[index]._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(renderpassInfo->_attachments[index]._storeOp);
    }

    return new VulkanRenderPass(m_deviceInfo._device, descs);
}

Pipeline* VulkanGraphicContext::createPipeline(Pipeline::PipelineType type)
{
    if (type == Pipeline::PipelineType::PipelineType_Graphic)
    {
        return new VulkanGraphicPipeline(m_deviceInfo._device, m_renderpassManager);
    }

    ASSERT(false, "not supported");
    return nullptr;
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

bool VulkanGraphicContext::prepareDraw()
{
    return true;
}

VulkanCommandBuffer * VulkanGraphicContext::getOrCreateAndStartCommandBuffer(CommandTargetType type)
{
    VulkanCommandBuffer * currentBuffer = m_currentContextState._currentCmdBuffer[type];
    if (!currentBuffer)
    {
        currentBuffer = m_cmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
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


VulkanGraphicContext::CurrentContextState::CurrentContextState()
{
    _currentCmdBuffer[CommandTargetType::CmdDrawBuffer] = nullptr;
    _currentCmdBuffer[CommandTargetType::CmdUploadBuffer] = nullptr;

    _currentRenderpass = nullptr;
    _currentFramebuffer = nullptr;
    _currentPipeline = nullptr;
}

void VulkanGraphicContext::CurrentContextState::invalidateState()
{
}

void VulkanGraphicContext::CurrentContextState::invalidateCommandBuffer(CommandTargetType type)
{
    _currentCmdBuffer[type] = nullptr;
    if (type == CommandTargetType::CmdDrawBuffer)
    {
        _stateCallbacks.clear();
    }
}

VulkanCommandBuffer * VulkanGraphicContext::CurrentContextState::getAcitveBuffer(CommandTargetType type)
{
    VulkanCommandBuffer * currentBuffer = _currentCmdBuffer[type];
    ASSERT(currentBuffer, "nullptr");
    return currentBuffer;
}

bool VulkanGraphicContext::CurrentContextState::isCurrentBufferAcitve(CommandTargetType type) const
{
    return _currentCmdBuffer[type] != nullptr;
}

bool VulkanGraphicContext::CurrentContextState::isCurrentFramebuffer(const Framebuffer * framebuffer) const
{
    return _currentFramebuffer ==framebuffer;
}

bool VulkanGraphicContext::CurrentContextState::isCurrentPipeline(const Pipeline * pipeline) const
{
    return _currentPipeline == pipeline;
}

bool VulkanGraphicContext::CurrentContextState::setDynamicState(VkDynamicState state, const std::function<void()>& callback)
{
    auto iter = _stateCallbacks.emplace(state, callback);
    if (iter.second)
    {
        return true;
    }

    return false;
}

bool VulkanGraphicContext::CurrentContextState::isCurrentRenderPass(const RenderPass* pass) const
{
    return _currentRenderpass == pass;
}


} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
