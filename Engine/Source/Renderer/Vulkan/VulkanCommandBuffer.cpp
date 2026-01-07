#include "VulkanCommandBuffer.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanResource.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanImage.h"
#   include "VulkanSemaphore.h"
#   include "VulkanSwapchain.h"

#   include "VulkanBuffer.h"
#   include "VulkanRenderpass.h"
#   include "VulkanFramebuffer.h"
#   include "VulkanCommandBufferManager.h"
#   include "VulkanGraphicPipeline.h"
#   include "VulkanComputePipeline.h"
#   include "VulkanFence.h"
//#include "VulkanQueryPool.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, CommandBufferLevel level, VulkanCommandBuffer* primaryBuffer) noexcept
    : m_device(*device)

    , m_pool(VK_NULL_HANDLE)
    , m_commands(VK_NULL_HANDLE)

    , m_level(level)
    , m_status(CommandBufferStatus::Invalid)
    , m_queueIndex(~0U)

    , m_fence(V3D_NEW(VulkanFence, memory::MemoryLabel::MemoryRenderCore)(&m_device))
    , m_primaryBuffer(nullptr)

    , m_isInsideRenderPass(false)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer constructor %llx", this);
#endif //VULKAN_DEBUG
    if (m_level == CommandBufferLevel::SecondaryBuffer)
    {
        ASSERT(primaryBuffer, "primaryBuffer is nullptr");
        m_primaryBuffer = primaryBuffer;
    }

    memset(&m_renderpassState, 0, sizeof(RenderPassState));

#if VULKAN_DEBUG_MARKERS
    m_debugName = "CommandBuffer";
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
#if VULKAN_DEBUG
    LOG_DEBUG("~VulkanCommandBuffer destructor %llx", this);
#endif //VULKAN_DEBUG
    //released form pool manager
    m_commands = VK_NULL_HANDLE;
    V3D_DELETE(m_fence, memory::MemoryLabel::MemoryRenderCore);

    m_primaryBuffer = nullptr;
    m_secondaryBuffers.clear();
}

void VulkanCommandBuffer::addSemaphore(VkPipelineStageFlags mask, VulkanSemaphore* semaphore)
{
    if (m_level == CommandBufferLevel::SecondaryBuffer) [[unlikely]]
    {
        ASSERT(m_primaryBuffer, "m_primaryBuffer is nullptr");
        m_primaryBuffer->addSemaphore(mask, semaphore);
    }

    ASSERT(std::find(m_semaphores.cbegin(), m_semaphores.cend(), semaphore) == m_semaphores.cend(), "added multiply times");
    m_stageMasks.push_back(mask);
    m_semaphores.push_back(semaphore);
}

void VulkanCommandBuffer::addSemaphores(VkPipelineStageFlags mask, const std::vector<VulkanSemaphore*>& semaphores)
{
    for (VulkanSemaphore* semaphore : semaphores)
    {
        VulkanCommandBuffer::addSemaphore(mask, semaphore);
    }
}

bool VulkanCommandBuffer::waitCompletion(u64 timeout)
{
    if (m_status == CommandBufferStatus::Finished)
    {
        return true;
    }

    if (m_status == CommandBufferStatus::Submit)
    {
        timeout = (timeout == 0) ? u64(~0ULL) : timeout;
        VkFence vkFence = m_fence->getHandle();
        VkResult result = VulkanWrapper::WaitForFences(m_device.getDeviceInfo()._device, 1, &vkFence, VK_TRUE, timeout);
        if (result != VK_SUCCESS)
        {
            LOG_WARNING("VulkanCommandBuffer::waitComplete vkWaitForFences. Error %s", ErrorString(result).c_str());
            return false;
        }
        m_status = CommandBufferStatus::Finished;
        m_fence->incrementValue();

        result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
        ASSERT(result == VK_SUCCESS, "must be reseted");

        return true;
    }

    ASSERT(false, "not started");
    return false;
}

void VulkanCommandBuffer::refreshFenceStatus()
{
    if (m_status == CommandBufferStatus::Submit) [[likely]]
    {
        if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
        {
            VkFence vkFence = m_fence->getHandle();
            VkResult result = VulkanWrapper::GetFenceStatus(m_device.getDeviceInfo()._device, vkFence);
            if (result == VK_SUCCESS) [[likely]]
            {
                m_status = CommandBufferStatus::Finished;
                m_fence->incrementValue();

                result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
                ASSERT(result == VK_SUCCESS, "must be reseted");
            }
            else if (result == VK_NOT_READY && VulkanCommandBuffer::isSafeFrame(m_renderpassState._activeSwapchain ? m_renderpassState._activeSwapchain->getCurrentFrameIndex() : 0))
            {
                VkResult result = VulkanWrapper::WaitForFences(m_device.getDeviceInfo()._device, 1, &vkFence, VK_TRUE, u64(~0ULL));
                ASSERT(result == VK_SUCCESS, "must be finished");

                m_status = CommandBufferStatus::Finished;
                m_fence->incrementValue();

                result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
                ASSERT(result == VK_SUCCESS, "must be reseted");
            }
        }
        else
        {
            ASSERT(false, "not supported");
        }
    }
}

void VulkanCommandBuffer::init(Device::DeviceMask queueMask, VkCommandPool pool, VkCommandBuffer buffer)
{
    m_pool = pool;
    m_commands = buffer;
    m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;
    m_queueIndex = queueMask >> 1;

    //Has crash inside renderdoc
#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_commands);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS
}

void VulkanCommandBuffer::resetStatus()
{
    m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;
    memset(&m_renderpassState, 0, sizeof(RenderPassState));

    m_stageMasks.clear();
    m_semaphores.clear();

    for (auto& secondaryBuffer : m_secondaryBuffers)
    {
        secondaryBuffer->resetStatus();
    }

#if VULKAN_DEBUG
    m_resources.clear();
#endif //VULKAN_DEBUG
}

bool VulkanCommandBuffer::isSafeFrame(u64 frame) const
{
    if (m_renderpassState._activeSwapchain)
    {
        u64 countSwapchains = m_renderpassState._activeSwapchain->getSwapchainImageCount();
        return (m_capturedFrameIndex + countSwapchains) < frame;
    }

    return false;
}

void VulkanCommandBuffer::beginCommandBuffer()
{
    ASSERT(m_status == CommandBufferStatus::Ready, "invalid state");

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
    }
    else
    {
        ASSERT(false, "secondary not implemented");
        VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};

        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;
    }

    VkResult result = VulkanWrapper::BeginCommandBuffer(m_commands, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBuffer::beginCommandBuffer vkBeginCommandBuffer. Error %s", ErrorString(result).c_str());
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsLabelEXT debugUtilsLabel = {};
        debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        debugUtilsLabel.pNext = nullptr;
        debugUtilsLabel.pLabelName = m_debugName.c_str();
        debugUtilsLabel.color[0] = 1.0f;
        debugUtilsLabel.color[1] = 1.0f;
        debugUtilsLabel.color[2] = 1.0f;
        debugUtilsLabel.color[3] = 1.0f;

        VulkanWrapper::CmdBeginDebugUtilsLabel(m_commands, &debugUtilsLabel);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_status = CommandBufferStatus::Begin;
}

void VulkanCommandBuffer::endCommandBuffer()
{
#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VulkanWrapper::CmdEndDebugUtilsLabel(m_commands);
    }
#endif //VULKAN_DEBUG_MARKERS

#if TRACE_PROFILER_GPU_ENABLE
    ASSERT(m_tracyContext, "must be valid");
    TracyVkCollect(m_tracyContext, m_commands);
#endif

    ASSERT(m_status == CommandBufferStatus::Begin, "invalid state");
    VulkanWrapper::EndCommandBuffer(m_commands);

    m_status = CommandBufferStatus::End;
}

void VulkanCommandBuffer::cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, const std::vector<VkClearValue>& clearValues)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(m_level == CommandBufferLevel::PrimaryBuffer, "primary only");
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass area [%d, %d, %u, %u], framebuffer area [width %u, height %u]", area.offset.x, area.offset.y, area.extent.width, area.extent.height, framebuffer->getArea().m_width, framebuffer->getArea().m_height);
#endif

    VulkanCommandBuffer::captureResource(pass);
    VulkanCommandBuffer::captureResource(framebuffer);

    m_renderpassState._renderpass = pass;
    m_renderpassState._framebuffer = framebuffer;

    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {
        auto& [vkImage, subresource] = framebuffer->getImage(index);
        VulkanCommandBuffer::captureResource(vkImage);

        VkImageLayout currentLayout = m_resourceStates.getLayout(vkImage, subresource);
        VkImageLayout initialLayout = pass->getAttachmentDescription(index)._initialLayout;
        if (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            VulkanCommandBuffer::cmdPipelineBarrier(vkImage, subresource, currentLayout, initialLayout);
        }

        VkImageLayout layout = pass->getAttachmentLayout<0>(index);
        m_resourceStates.setLayout(vkImage, layout, subresource);

        if (vkImage->hasUsageFlag(TextureUsage_Backbuffer))
        {
            m_renderpassState._activeSwapchain = VulkanImage::getSwapchainFromImage(vkImage);
        }

#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass framebuffer image (width %u, height %u)", vkImage->getSize().width, vkImage->getSize().height);
#endif
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = pass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea = framebuffer->getRenderArea();
    renderPassBeginInfo.clearValueCount = framebuffer->getCountImage();
    renderPassBeginInfo.pClearValues = clearValues.data();

#ifdef PLATFORM_ANDROID
#   ifdef VK_QCOM_render_pass_transform
    VkRenderPassTransformBeginInfoQCOM renderPassTransformBeginInfoQCOM = {};
    if (m_device->m_deviceCaps._renderpassTransformQCOM && pass->isDrawingToSwapchain())
    {
        VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VulkanContext*>(m_context)->getSwapchain()->getTransformFlag();
        if (preTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            renderPassTransformBeginInfoQCOM.sType = m_device->m_deviceCaps._fixRenderPassTransformQCOMDriverIssue ? (VkStructureType)1000282000 : (VkStructureType)VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM;
            renderPassTransformBeginInfoQCOM.pNext = nullptr;
            renderPassTransformBeginInfoQCOM.transform = preTransform;

            renderPassBeginInfo.pNext = &renderPassTransformBeginInfoQCOM;
            LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass VkRenderPassTransformBeginInfoQCOM, transform %d", preTransform);
        }
    }
#   endif //VK_QCOM_render_pass_transform
#endif //PLATFORM_ANDROID

#if VULKAN_DEBUG_MARKERS && 0
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsLabelEXT debugUtilsLabel = {};
        debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        debugUtilsLabel.pNext = nullptr;
        debugUtilsLabel.pLabelName = framebuffer->m_debugName.c_str();
        debugUtilsLabel.color[0] = 0.7f;
        debugUtilsLabel.color[1] = 0.7f;
        debugUtilsLabel.color[2] = 0.7f;
        debugUtilsLabel.color[3] = 1.0f;

        VulkanWrapper::CmdBeginDebugUtilsLabel(m_commands, &debugUtilsLabel);
    }
#endif //VULKAN_DEBUG_MARKERS

    if (m_device.getVulkanDeviceCaps()._supportRenderpass2)
    {
        VkSubpassBeginInfoKHR subpassBeginInfo = {};
        subpassBeginInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR;
        subpassBeginInfo.pNext = nullptr;
        subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::CmdBeginRenderPass2 area (width %u, height %u)", renderPassBeginInfo.renderArea.extent.width, renderPassBeginInfo.renderArea.extent.height);
#endif
        VulkanWrapper::CmdBeginRenderPass2(m_commands, &renderPassBeginInfo, &subpassBeginInfo);
    }
    else
    {
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::CmdBeginRenderPass area (width %u, height %u)", renderPassBeginInfo.renderArea.extent.width, renderPassBeginInfo.renderArea.extent.height);
#endif
        VulkanWrapper::CmdBeginRenderPass(m_commands, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    m_isInsideRenderPass = true;
}

void VulkanCommandBuffer::cmdEndRenderPass()
{
    ASSERT(m_status == CommandBufferStatus::Begin && m_isInsideRenderPass, "invalid state");
    if (m_device.getVulkanDeviceCaps()._supportRenderpass2)
    {
        VkSubpassEndInfoKHR subpassEndInfo = {};
        subpassEndInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR;
        subpassEndInfo.pNext = nullptr;

        VulkanWrapper::CmdEndRenderPass2(m_commands, &subpassEndInfo);
    }
    else
    {
        VulkanWrapper::CmdEndRenderPass(m_commands);
    }

    const VulkanRenderPass* pass = m_renderpassState._renderpass;
    const VulkanFramebuffer* framebuffer = m_renderpassState._framebuffer;

    ASSERT(pass && framebuffer, "nullptr");
    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {
        auto& [vkImage, subresource] = framebuffer->getImage(index);
        VkImageLayout layout = m_renderpassState._renderpass->getAttachmentLayout<1>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        m_resourceStates.setLayout(vkImage, layout, subresource);
    }

#if VULKAN_DEBUG_MARKERS && 0
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VulkanWrapper::CmdEndDebugUtilsLabel(m_commands);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_isInsideRenderPass = false;
}

void VulkanCommandBuffer::cmdBeginRendering(const RenderPassDesc& passDesc, const FramebufferDesc& framebufferDesc, const std::vector<VkClearValue>& clearValues)
{
    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    for (u32 color = 0; color < passDesc._countColorAttachment; ++color)
    {
        const AttachmentDesc& description = passDesc._attachmentsDesc[color];
        auto& [texture, subresource] = framebufferDesc._imageViews[color];
        VulkanImage* vkImage = nullptr;
        if (texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
        {
            VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(objectFromHandle<Swapchain>(texture->getTextureHandle()));
            vkImage = swapchain->getCurrentSwapchainImage();
            m_renderpassState._activeSwapchain = VulkanImage::getSwapchainFromImage(vkImage);
        }
        else
        {
            vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(texture->getTextureHandle()));
        }
        ASSERT(vkImage, "nullptr");

        VkImageLayout oldLayout = m_resourceStates.getLayout(vkImage, subresource);
        VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._transition);
        VulkanCommandBuffer::cmdPipelineBarrier(vkImage, subresource, oldLayout, newLayout);

        VkRenderingAttachmentInfo renderingAttachmentInfo = {};
        renderingAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        renderingAttachmentInfo.pNext = nullptr;
        renderingAttachmentInfo.loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._loadOp);
        renderingAttachmentInfo.storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._storeOp);
        renderingAttachmentInfo.imageView = vkImage->getImageView(subresource, VK_IMAGE_ASPECT_COLOR_BIT);
        renderingAttachmentInfo.imageLayout = newLayout;
        renderingAttachmentInfo.clearValue.color = clearValues[color].color;
        if (texture->hasUsageFlag(TextureUsage::TextureUsage_Resolve))
        {
            renderingAttachmentInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            renderingAttachmentInfo.resolveImageView = vkImage->getResolveImage()->getImageView(subresource, VK_IMAGE_ASPECT_COLOR_BIT);
            renderingAttachmentInfo.resolveImageLayout = newLayout;
        }

        colorAttachments.push_back(renderingAttachmentInfo);
    }

    VkRenderingAttachmentInfo depthAttachment = {};
    VkRenderingAttachmentInfo stencilAttachment = {};
    const AttachmentDesc& depthStencilDescription = passDesc._attachmentsDesc.back();
    if (passDesc._hasDepthStencilAttachment)
    {
        auto& [image, subresource] = framebufferDesc._imageViews.back();
        VulkanImage* vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(image->getTextureHandle()));
        ASSERT(vkImage, "nullptr");

        VkImageLayout oldLayout = m_resourceStates.getLayout(vkImage, subresource);
        VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(depthStencilDescription._transition);
        VulkanCommandBuffer::cmdPipelineBarrier(vkImage, subresource, oldLayout, newLayout);

        if (VulkanImage::isDepthFormat(VulkanImage::convertImageFormatToVkFormat(depthStencilDescription._format)))
        {
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.pNext = nullptr;
            depthAttachment.loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(depthStencilDescription._loadOp);
            depthAttachment.storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(depthStencilDescription._storeOp);
            depthAttachment.imageView = vkImage->getImageView(subresource);
            depthAttachment.imageLayout = newLayout;
            depthAttachment.clearValue.depthStencil = { framebufferDesc._clearDepthValue, framebufferDesc._clearStencilValue };
            if (image->hasUsageFlag(TextureUsage::TextureUsage_Resolve))
            {
                depthAttachment.resolveMode = VK_RESOLVE_MODE_MAX_BIT;
                depthAttachment.resolveImageView = vkImage->getResolveImage()->getImageView(subresource);
                depthAttachment.resolveImageLayout = newLayout;
            }
        }

        if (VulkanImage::isStencilFormat(VulkanImage::convertImageFormatToVkFormat(depthStencilDescription._format)))
        {
            stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            stencilAttachment.pNext = nullptr;
            stencilAttachment.loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(depthStencilDescription._stencilLoadOp);
            stencilAttachment.storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(depthStencilDescription._stencilStoreOp);
            stencilAttachment.imageView = vkImage->getImageView(subresource);
            stencilAttachment.imageLayout = newLayout;
            stencilAttachment.clearValue.depthStencil = { framebufferDesc._clearDepthValue, framebufferDesc._clearStencilValue };
            if (image->hasUsageFlag(TextureUsage::TextureUsage_Resolve))
            {
                stencilAttachment.resolveMode = VK_RESOLVE_MODE_MAX_BIT;
                stencilAttachment.resolveImageView = vkImage->getResolveImage()->getImageView(subresource);
                stencilAttachment.resolveImageLayout = newLayout;
            }
        }
    }

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.flags = 0;
    renderingInfo.renderArea = { {0, 0 }, framebufferDesc._renderArea._width, framebufferDesc._renderArea._height };
    renderingInfo.viewMask = passDesc._viewsMask;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<u32>(colorAttachments.size());
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.pDepthAttachment = (passDesc._hasDepthStencilAttachment && VulkanImage::isDepthFormat(VulkanImage::convertImageFormatToVkFormat(depthStencilDescription._format))) ? &depthAttachment : nullptr;
    renderingInfo.pStencilAttachment = (passDesc._hasDepthStencilAttachment && VulkanImage::isStencilFormat(VulkanImage::convertImageFormatToVkFormat(depthStencilDescription._format))) ? &stencilAttachment : nullptr;

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::cmdBeginRendering area (width %u, height %u)", renderingInfo.renderArea.extent.width, renderingInfo.renderArea.extent.height);
#endif
    VulkanWrapper::CmdBeginRendering(m_commands, &renderingInfo);
    m_isInsideRenderPass = true;
}

void VulkanCommandBuffer::cmdEndRendering(const RenderPassDesc& passDesc, const FramebufferDesc& framebufferDesc)
{
    ASSERT(m_status == CommandBufferStatus::Begin && m_isInsideRenderPass, "invalid state");

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::cmdEndRendering");
#endif
    VulkanWrapper::CmdEndRendering(m_commands);
    m_isInsideRenderPass = false;

    for (u32 color = 0; color < passDesc._countColorAttachment; ++color)
    {
        const AttachmentDesc& description = passDesc._attachmentsDesc[color];
        auto& [texture, subresource] = framebufferDesc._imageViews[color];
        VulkanImage* vkImage = nullptr;
        if (texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
        {
            VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(objectFromHandle<Swapchain>(texture->getTextureHandle()));
            vkImage = swapchain->getCurrentSwapchainImage();
            m_renderpassState._activeSwapchain = VulkanImage::getSwapchainFromImage(vkImage);
        }
        else
        {
            vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(texture->getTextureHandle()));
        }
        ASSERT(vkImage, "nullptr");

        VkImageLayout oldLayout = m_resourceStates.getLayout(vkImage, subresource);
        VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._finalTransition);
        VulkanCommandBuffer::cmdPipelineBarrier(vkImage, subresource, oldLayout, newLayout);
    }

    if (passDesc._hasDepthStencilAttachment)
    {
        const AttachmentDesc& description = passDesc._attachmentsDesc.back();
        auto& [image, subresource] = framebufferDesc._imageViews.back();
        VulkanImage* vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(image->getTextureHandle()));
        ASSERT(vkImage, "nullptr");

        VkImageLayout oldLayout = m_resourceStates.getLayout(vkImage, subresource);
        VkImageLayout newLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._finalTransition);
        VulkanCommandBuffer::cmdPipelineBarrier(vkImage, subresource, oldLayout, newLayout);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
