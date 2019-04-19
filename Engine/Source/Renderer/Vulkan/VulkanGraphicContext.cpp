#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDescriptorSet.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanStagingBuffer.h"
#include "VulkanContextState.h"
#include "VulkanUnifromBuffer.h"

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
    //VK_DYNAMIC_STATE_STENCIL_REFERENCE
};

VulkanGraphicContext::VulkanGraphicContext(const platform::Window * window) noexcept
    : m_deviceCaps(*VulkanDeviceCaps::getInstance())
    , m_swapchain(nullptr)
    , m_cmdBufferManager(nullptr)

    , m_stagingBufferManager(nullptr)
    , m_uniformBufferManager(nullptr)
    , m_descriptorSetManager(nullptr)

    , m_imageMemoryManager(nullptr)
    , m_bufferMemoryManager(nullptr)

    , m_renderpassManager(nullptr)
    , m_framebuferManager(nullptr)
    , m_pipelineManager(nullptr)
    , m_samplerManager(nullptr)

    , m_currentContextStateNEW(nullptr)

    , m_window(window)
{
    LOG_DEBUG("VulkanGraphicContext created this %llx", this);

    m_renderType = RenderType::VulkanRender;
    memset(&m_deviceInfo, 0, sizeof(DeviceInfo));

#if VULKAN_DUMP
    VulkanDump::getInstance()->clearFile(VULKAN_DUMP_FILE);
#endif
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
    ASSERT(!m_samplerManager, "m_samplerManager not nullptr");

    ASSERT(!m_swapchain, "m_swapchain not nullptr");
    ASSERT(!m_cmdBufferManager, "m_cmdBufferManager not nullptr");

    ASSERT(m_deviceInfo._device == VK_NULL_HANDLE, "Device is not nullptr");
    ASSERT(m_deviceInfo._instance == VK_NULL_HANDLE, "Instance is not nullptr");
 }

void VulkanGraphicContext::beginFrame()
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpFrameNumber(m_frameCounter);
#endif

    u32 index = m_swapchain->acquireImage();
    m_currentContextStateNEW->updateSwapchainIndex(index);
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::beginFrame %llu, image index %u", m_frameCounter, index);
#endif //VULKAN_DEBUG
    ASSERT(!m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "buffer exist");
    VulkanCommandBuffer* drawBuffer = m_currentContextState._currentCmdBuffer[CommandTargetType::CmdDrawBuffer] = m_cmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);

    if (drawBuffer->getStatus() != VulkanCommandBuffer::CommandBufferStatus::Ready)
    {
        LOG_ERROR("VulkanGraphicContext::beginFrame CommandBufferStatus is Invalid");
    }
    drawBuffer->beginCommandBuffer();
    //drawBuffer->cmdPipelineBarrier(m_swapchain->getBackbuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanGraphicContext::endFrame()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::endFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG
    if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdUploadBuffer))
    {
        VulkanCommandBuffer* uploadBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdUploadBuffer);
        if (uploadBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Begin)
        {
            uploadBuffer->endCommandBuffer();
        }
        m_cmdBufferManager->submit(uploadBuffer, VK_NULL_HANDLE);
        //uploadBuffer->waitComplete();
        m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdUploadBuffer);
    }

    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "m_currentDrawBuffer is nullptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        ASSERT(m_currentContextStateNEW->getCurrentPipeline(), "nullptr");
        drawBuffer->cmdEndRenderPass();
    }
    //drawBuffer->cmdPipelineBarrier(m_swapchain->getBackbuffer(),VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    drawBuffer->endCommandBuffer();
    m_cmdBufferManager->submit(drawBuffer, VK_NULL_HANDLE);
    m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdDrawBuffer);
}

void VulkanGraphicContext::presentFrame()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::presentFrame %llu", m_frameCounter);
#endif //VULKAN_DEBUG
    std::vector<VkSemaphore> semaphores;
    m_swapchain->present(m_queueList[0], semaphores);

    m_cmdBufferManager->updateCommandBuffers();
    m_uniformBufferManager->updateUniformBuffers();
    if (m_deviceCaps.useStagingBuffers)
    {
        m_stagingBufferManager->destroyStagingBuffers();
    }
    m_currentContextStateNEW->invalidateDescriptorSetsState();

    m_frameCounter++;
}

void VulkanGraphicContext::submit(bool wait)
{
    if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdUploadBuffer))
    {
        VulkanCommandBuffer* uploadBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdUploadBuffer);
        if (uploadBuffer->getStatus() == VulkanCommandBuffer::CommandBufferStatus::Begin)
        {
            uploadBuffer->endCommandBuffer();
        }
        m_cmdBufferManager->submit(uploadBuffer, VK_NULL_HANDLE);
        uploadBuffer->waitComplete();
        m_currentContextState.invalidateCommandBuffer(CommandTargetType::CmdUploadBuffer);
    }

    if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer))
    {
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

#if VULKAN_DUMP
    VulkanDump::getInstance()->flushToFile(VULKAN_DUMP_FILE);
#endif
}

void VulkanGraphicContext::clearBackbuffer(const core::Vector4D & color)
{
    ASSERT(m_swapchain, "m_swapchain is nullptr");
    m_swapchain->getBackbuffer()->clear(this, color);
}

void VulkanGraphicContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::setViewport [%u, %u; %u, %u]", viewport.getLeftX(), viewport.getTopY(), viewport.getWidth(), viewport.getHeight());
#endif //VULKAN_DEBUG
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
        m_currentContextStateNEW->setDynamicState(VK_DYNAMIC_STATE_VIEWPORT, std::bind(&VulkanCommandBuffer::cmdSetViewport, drawBuffer, viewports));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanGraphicContext::setScissor(const core::Rect32 & scissor)
{
    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::setScissor [%u, %u; %u, %u]", scissor.getLeftX(), scissor.getTopY(), scissor.getWidth(), scissor.getHeight());
#endif //VULKAN_DEBUG
    if (VulkanGraphicContext::isDynamicState(VK_DYNAMIC_STATE_SCISSOR))
    {
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.getLeftX(), scissor.getTopY() };
        vkScissor.extent = { static_cast<u32>(scissor.getWidth()), static_cast<u32>(scissor.getHeight()) };

        std::vector<VkRect2D> scissors = { vkScissor };

        VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
        m_currentContextStateNEW->setDynamicState(VK_DYNAMIC_STATE_SCISSOR, std::bind(&VulkanCommandBuffer::cmdSetScissor, drawBuffer, scissors));
    }
    else
    {
        ASSERT(false, "static state");
    }
}

void VulkanGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
    ASSERT(renderpassInfo && framebufferInfo, "nullptr");

    RenderPass* renderpass = m_renderpassManager->acquireRenderPass(renderpassInfo->_value._desc);
    ASSERT(renderpass, "renderpass is nullptr");
    renderpassInfo->_tracker->attach(renderpass);
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);

    std::vector<VulkanFramebuffer*> vkFramebuffer;
    bool swapchainPresnet = std::find(framebufferInfo->_images.cbegin(), framebufferInfo->_images.cend(), nullptr) != framebufferInfo->_images.cend();
    if (swapchainPresnet)
    {
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

            Framebuffer* framebuffer = m_framebuferManager->acquireFramebuffer(renderpass, images, framebufferInfo->_clearInfo._size);
            ASSERT(framebuffer, "framebuffer is nullptr");
            framebufferInfo->_tracker->attach(framebuffer);
            vkFramebuffer.push_back(static_cast<VulkanFramebuffer*>(framebuffer));
        }
    }
    else
    {
        Framebuffer* framebuffer = m_framebuferManager->acquireFramebuffer(renderpass, framebufferInfo->_images, framebufferInfo->_clearInfo._size);
        ASSERT(framebuffer, "framebuffer is nullptr");
        framebufferInfo->_tracker->attach(framebuffer);
        vkFramebuffer.push_back(static_cast<VulkanFramebuffer*>(framebuffer));
    }

    if (!m_currentContextStateNEW->isCurrentRenderPass(vkRenderpass) || !m_currentContextStateNEW->isCurrentFramebuffer(vkFramebuffer.back()) /*|| clearInfo*/)
    {
        if (m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer))
        {
            VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
            if (drawBuffer->isInsideRenderPass())
            {
                drawBuffer->cmdEndRenderPass();
            }
        }

        m_currentContextStateNEW->setCurrentRenderPass(vkRenderpass);
        m_currentContextStateNEW->setCurrentFramebuffer(vkFramebuffer);

        VkRect2D area;
        area.offset = { 0, 0 };
        area.extent = { framebufferInfo->_clearInfo._size.width, framebufferInfo->_clearInfo._size.height };


        u32 countClearValues = static_cast<u32>(framebufferInfo->_clearInfo._color.size()) + (renderpassInfo->_value._desc._hasDepthStencilAttahment ? 1 : 0);
        std::vector<VkClearValue> clearValues(countClearValues);
        for (u32 index = 0; index < framebufferInfo->_clearInfo._color.size(); ++index)
        {
            clearValues[index] = { framebufferInfo->_clearInfo._color[index].x, framebufferInfo->_clearInfo._color[index].y, framebufferInfo->_clearInfo._color[index].z, framebufferInfo->_clearInfo._color[index].w };
        }

        if (renderpassInfo->_value._desc._hasDepthStencilAttahment)
        {
            clearValues.back().depthStencil = { framebufferInfo->_clearInfo._depth, framebufferInfo->_clearInfo._stencil };
        }
        m_currentContextStateNEW->setClearValues(area, clearValues);

        //TODO: start before draw command
        //drawBuffer->cmdBeginRenderpass(m_currentContextStateNEW->getCurrentRenderpass(), m_currentContextStateNEW->getCurrentFramebuffer(), area, clearValues);
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
    if (m_currentContextStateNEW->isCurrentFramebuffer(vkFramebuffer) || vkFramebuffer->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }

    m_framebuferManager->removeFramebuffer(framebuffer);
}

void VulkanGraphicContext::removeRenderPass(RenderPass * renderpass)
{
    VulkanRenderPass* vkRenderpass = static_cast<VulkanRenderPass*>(renderpass);
    if (m_currentContextStateNEW->isCurrentRenderPass(vkRenderpass) || vkRenderpass->isCaptured())
    {
        ASSERT(false, "not implementing");
        //delayed delete
    }

    m_renderpassManager->removeRenderPass(renderpass);
}

void VulkanGraphicContext::invalidateRenderPass()
{
    ASSERT(m_currentContextStateNEW->getCurrentRenderpass(), "nuulptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdEndRenderPass();
    }
}

void VulkanGraphicContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

    Pipeline* pipeline = m_pipelineManager->acquireGraphicPipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
    pipelineInfo->_tracker->attach(pipeline);
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicContext::setPipeline %xll", pipeline);
#endif //VULKAN_DEBUG

    VulkanGraphicPipeline* vkPipeline = static_cast<VulkanGraphicPipeline*>(pipeline);
    m_pendingState.setPendingPipeline(vkPipeline);
}

void VulkanGraphicContext::removePipeline(Pipeline * pipeline)
{
    VulkanGraphicPipeline* vkPipeline = static_cast<VulkanGraphicPipeline*>(pipeline);;
    if (m_currentContextStateNEW->isCurrentPipeline(vkPipeline) || vkPipeline->isCaptured())
    {
        ASSERT(false, "not implementing");
        //TODO: delayed delete
    }

    m_pipelineManager->removePipeline(pipeline);
}

Image * VulkanGraphicContext::createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 layers, u32 mipLevels, TextureUsageFlags flags)
{
    VkImageType vkType = VulkanImage::convertTextureTargetToVkImageType(target);
    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };

    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkType, vkFormat, vkExtent, layers, mipLevels, VK_IMAGE_TILING_OPTIMAL, flags);
}

//Image * VulkanGraphicContext::createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags) const
//{
//    VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat(format);
//    VkExtent3D vkExtent = { dimension.width, dimension.height, dimension.depth };
//    VkSampleCountFlagBits vkSamples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(samples);
//
//    return new VulkanImage(m_imageMemoryManager, m_deviceInfo._device, vkFormat, vkExtent, vkSamples);
//}

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

void VulkanGraphicContext::removeSampler(Sampler * sampler)
{
    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
    if (vkSampler->isCaptured())
    {
        //delayed destoy
        ASSERT(false, "not implementing");
    }
    else
    {
        m_samplerManager->removeSampler(sampler);
    }
}

void VulkanGraphicContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::UniformBuffer& bufferData = info._uniformBuffers[bindIndex];
    if (offset == 0)
    {
        ASSERT(bufferData._size * bufferData._array == size, "different size");
    }

    m_currentContextStateNEW->updateConstantBuffer(bindIndex, bufferData, offset, size, data);
}

void VulkanGraphicContext::transitionImages(const std::vector<Image*>& images, TransitionOp transition, s32 layer)
{
    ASSERT(false, "impl");
}

void VulkanGraphicContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    bool changed = m_currentContextStateNEW->setCurrentVertexBuffers(desc);

    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (prepareDraw(drawBuffer))
    {
        //if (changed)
        {
            const StreamBufferDescription& desc = m_currentContextStateNEW->getStreamBufferDescription();
            if (!desc._vertices.empty())
            {
                drawBuffer->cmdBindVertexBuffers(0, static_cast<u32>(desc._vertices.size()), desc._vertices, desc._offsets);
            }
        }
        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDraw(firstVertex, vertexCount, firstInstance, instanceCount);
    }

    //m_currentContextStateNEW->invalidateDescriptorSetsState();
}

void VulkanGraphicContext::drawIndexed(const StreamBufferDescription & desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
    bool changed = m_currentContextStateNEW->setCurrentVertexBuffers(desc);

    ASSERT(m_currentContextState.isCurrentBufferAcitve(CommandTargetType::CmdDrawBuffer), "nullptr");
    VulkanCommandBuffer* drawBuffer = m_currentContextState.getAcitveBuffer(CommandTargetType::CmdDrawBuffer);
    if (prepareDraw(drawBuffer))
    {
        //if (changed)
        {
            const StreamBufferDescription& descBuff = m_currentContextStateNEW->getStreamBufferDescription();
            ASSERT(descBuff._indices, "nullptr");
            VulkanBuffer* indexBuffer = static_cast<VulkanBuffer*>(descBuff._indices);
            drawBuffer->cmdBindIndexBuffers(indexBuffer, descBuff._indicesOffet, (descBuff._indexType == StreamIndexBufferType::IndexType_16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

            ASSERT(!descBuff._vertices.empty(), "empty");
            drawBuffer->cmdBindVertexBuffers(0, static_cast<u32>(descBuff._vertices.size()), descBuff._vertices, descBuff._offsets);
        }
        ASSERT(drawBuffer->isInsideRenderPass(), "not inside renderpass");
        drawBuffer->cmdDrawIndexed(firstIndex, indexCount, firstInstance, instanceCount, 0);
    }
}

void VulkanGraphicContext::bindImage(const Shader * shader, u32 bindIndex, const Image * image)
{
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::SampledImage& sampledData = info._sampledImages[bindIndex];

    m_currentContextStateNEW->bindTexture(vkImage, nullptr, 1, sampledData);
}

void VulkanGraphicContext::bindSampler(const Shader * shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
    ASSERT(false, "not impl");
    //TODO:
}

void VulkanGraphicContext::bindSampledImage(const Shader * shader, u32 bindIndex, const Image * image, const Sampler::SamplerInfo* samplerInfo)
{
    ASSERT(image && samplerInfo, "nullptr");
    const VulkanImage* vkImage = static_cast<const VulkanImage*>(image);

    Sampler* sampler = m_samplerManager->acquireSampler(samplerInfo->_value._desc);
    ASSERT(sampler, "nullptr");
    samplerInfo->_tracker->attach(sampler);
    const VulkanSampler* vkSampler = static_cast<const VulkanSampler*>(sampler);

    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    const Shader::SampledImage& sampledData = info._sampledImages[bindIndex];

    m_currentContextStateNEW->bindTexture(vkImage, vkSampler, 0, sampledData);
}

const DeviceCaps* VulkanGraphicContext::getDeviceCaps() const
{
    return &m_deviceCaps;
}

VulkanStagingBufferManager * VulkanGraphicContext::getStagingManager()
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
    config._vsync = false; //TODO
    config._countSwapchainImages = 3;

    m_swapchain = new VulkanSwapchain(&m_deviceInfo, surface);
    if (!m_swapchain->create(config))
    {
        VulkanGraphicContext::destroy();
        LOG_FATAL("VulkanGraphicContext::createContext: Can not create VulkanSwapchain");
        return false;
    }

    m_backufferDescription._size = config._size;
    m_backufferDescription._format = VulkanImage::convertVkImageFormatToFormat(m_swapchain->getSwapchainImage(0)->getFormat());

    if (m_deviceCaps.unifiedMemoryManager)
    {
        m_imageMemoryManager = new SimpleVulkanMemoryAllocator(m_deviceInfo._device);
        m_bufferMemoryManager = m_imageMemoryManager;
    }
    else
    {
        m_imageMemoryManager = new SimpleVulkanMemoryAllocator(m_deviceInfo._device);
        m_bufferMemoryManager = new SimpleVulkanMemoryAllocator(m_deviceInfo._device);
    }

    m_cmdBufferManager = new VulkanCommandBufferManager(&m_deviceInfo, m_queueList[0]);
    //m_currentContextState._currentDrawBuffer = m_drawCmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
    //ASSERT(m_currentContextState._currentDrawBuffer, "m_currentDrawBuffer is nullptr");

    if (m_deviceCaps.useStagingBuffers)
    {
        m_stagingBufferManager = new VulkanStagingBufferManager(m_deviceInfo._device);
    }
    m_uniformBufferManager = new VulkanUniformBufferManager(m_deviceInfo._device);
    m_descriptorSetManager = new VulkanDescriptorSetManager(m_deviceInfo._device);

    m_renderpassManager = new RenderPassManager(this);
    m_framebuferManager = new FramebufferManager(this);
    m_pipelineManager = new PipelineManager(this);
    m_samplerManager = new SamplerManager(this);

    m_currentContextStateNEW = new VulkanContextState(m_deviceInfo._device, m_descriptorSetManager, m_uniformBufferManager);

    return true;
}

void VulkanGraphicContext::destroy()
{
    //Called from game thread
    LOG_DEBUG("VulkanGraphicContext::destroy");

    VkResult result = VulkanWrapper::DeviceWaitIdle(m_deviceInfo._device);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanGraphicContext::destroy DeviceWaitIdle is failed. Error: %s", ErrorString(result).c_str());
        ASSERT(false, "error");
    }

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

    if (m_uniformBufferManager)
    {
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
        m_descriptorSetManager->clear();

        delete m_descriptorSetManager;
        m_descriptorSetManager = nullptr;
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

    if (m_samplerManager)
    {
        delete m_samplerManager;
        m_samplerManager = nullptr;
    }

    if (m_currentContextStateNEW)
    {
        delete m_currentContextStateNEW;
        m_currentContextStateNEW = nullptr;
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

RenderPass * VulkanGraphicContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
    u32 countAttachments = (renderpassDesc->_hasDepthStencilAttahment) ? renderpassDesc->_countColorAttachments + 1 : renderpassDesc->_countColorAttachments;
    std::vector<VulkanRenderPass::VulkanAttachmentDescription> descs(countAttachments);
    for (u32 index = 0; index < renderpassDesc->_countColorAttachments; ++index)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = descs[index];
        desc._format = VulkanImage::convertImageFormatToVkFormat(renderpassDesc->_attachments[index]._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(renderpassDesc->_attachments[index]._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(renderpassDesc->_attachments[index]._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(renderpassDesc->_attachments[index]._storeOp);
        desc._initialLayout = VulkanRenderPass::convertTransitionStateToImageLayout(renderpassDesc->_attachments[index]._initTransition);
        desc._finalLayout = VulkanRenderPass::convertTransitionStateToImageLayout(renderpassDesc->_attachments[index]._finalTransition);

        desc._swapchainImage = (renderpassDesc->_attachments[index]._internalTarget) ? true : false;
    }

    if (renderpassDesc->_hasDepthStencilAttahment)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = descs.back();
        desc._format = VulkanImage::convertImageFormatToVkFormat(renderpassDesc->_attachments.back()._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(renderpassDesc->_attachments.back()._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(renderpassDesc->_attachments.back()._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(renderpassDesc->_attachments.back()._storeOp);
        desc._stencilLoadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(renderpassDesc->_attachments.back()._stencilLoadOp);
        desc._stensilStoreOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(renderpassDesc->_attachments.back()._stencilStoreOp);
        desc._initialLayout = VulkanRenderPass::convertTransitionStateToImageLayout(renderpassDesc->_attachments.back()._initTransition);
        desc._finalLayout = VulkanRenderPass::convertTransitionStateToImageLayout(renderpassDesc->_attachments.back()._finalTransition);
        desc._swapchainImage = (renderpassDesc->_attachments.back()._internalTarget) ? true : false;
    }

    return new VulkanRenderPass(m_deviceInfo._device, descs);
}

Pipeline* VulkanGraphicContext::createPipeline(Pipeline::PipelineType type)
{
    if (type == Pipeline::PipelineType::PipelineType_Graphic)
    {
        return new VulkanGraphicPipeline(m_deviceInfo._device, m_renderpassManager, m_descriptorSetManager);
    }

    ASSERT(false, "not supported");
    return nullptr;
}

Sampler * VulkanGraphicContext::createSampler()
{
    return new VulkanSampler(m_deviceInfo._device);
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
    instanceCreateInfo.pNext = nullptr; //VkDebugUtilsMessengerCreateInfoEXT
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
    switch (VulkanDebug::k_severityDebugLevel)
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
    VulkanDeviceCaps::s_enableExtensions = enabledExtensions;

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

bool VulkanGraphicContext::prepareDraw(VulkanCommandBuffer* drawBuffer)
{
    ASSERT(drawBuffer, "nullptr");

    ASSERT(m_currentContextStateNEW->getCurrentRenderpass(), "not bound");
    if (!drawBuffer->isInsideRenderPass())
    {
        drawBuffer->cmdBeginRenderpass(m_currentContextStateNEW->getCurrentRenderpass(), m_currentContextStateNEW->getCurrentFramebuffer(), m_currentContextStateNEW->m_renderPassArea, m_currentContextStateNEW->m_renderPassClearValues);
    }

    if (m_pendingState.isPipeline())
    {
        if (m_currentContextStateNEW->setCurrentPipeline(m_pendingState.takePipeline()))
        {
            drawBuffer->cmdBindPipeline(m_currentContextStateNEW->getCurrentPipeline());
        }
    }
    ASSERT(m_currentContextStateNEW->getCurrentPipeline(), "not bound");

    m_currentContextStateNEW->invokeDynamicStates();

    std::vector<VkDescriptorSet> sets;
    std::vector<u32> offsets;
    if (m_currentContextStateNEW->prepareDescriptorSets(drawBuffer, sets, offsets))
    {
        drawBuffer->cmdBindDescriptorSets(m_currentContextStateNEW->getCurrentPipeline(), 0, static_cast<u32>(sets.size()), sets, offsets);
    }

    return true;
}

VulkanCommandBuffer * VulkanGraphicContext::getOrCreateAndStartCommandBuffer(CommandTargetType type)
{
    VulkanCommandBuffer * currentBuffer = m_currentContextState._currentCmdBuffer[type];
    if (!currentBuffer)
    {
        currentBuffer = m_cmdBufferManager->acquireNewCmdBuffer(VulkanCommandBuffer::PrimaryBuffer);
        m_currentContextState._currentCmdBuffer[type] = currentBuffer;
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
}

void VulkanGraphicContext::CurrentContextState::invalidateState()
{
}

void VulkanGraphicContext::CurrentContextState::invalidateCommandBuffer(CommandTargetType type)
{
    _currentCmdBuffer[type] = nullptr;
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

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
