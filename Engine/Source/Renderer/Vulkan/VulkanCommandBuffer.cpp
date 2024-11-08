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

    , m_activeSwapchain(nullptr)
    , m_drawingToSwapchain(false)
    , m_isInsideRenderPass(false)
{
    LOG_DEBUG("VulkanCommandBuffer constructor %llx", this);

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
    LOG_DEBUG("~VulkanCommandBuffer destructor %llx", this);

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
    VulkanCommandBuffer::refreshFenceStatus();
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
                if (result != VK_SUCCESS)
                {
                    m_status = CommandBufferStatus::Invalid;
                    LOG_ERROR("VulkanCommandBuffer::refreshFenceStatus vkResetFences. Error %s", ErrorString(result).c_str());
                }

                m_stageMasks.clear();
                m_semaphores.clear();

                for (auto& secondaryBuffer : m_secondaryBuffers)
                {
                    secondaryBuffer->m_status = m_status;

                    secondaryBuffer->m_stageMasks.clear();
                    secondaryBuffer->m_semaphores.clear();
                }
                VulkanCommandBuffer::releaseResources();
            }
            else if (result == VK_NOT_READY && VulkanCommandBuffer::isSafeFrame(m_activeSwapchain ? m_activeSwapchain->getCurrentFrameIndex() : 0))
            {
                //TODO
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

void VulkanCommandBuffer::captureResource(VulkanResource* resource, u64 frame)
{
#if VULKAN_DEBUG
    m_resources.insert(resource);
#endif //VULKAN_DEBUG

    u64 capturedFrame = (frame == 0) ? m_capturedFrameIndex : frame;
    resource->markUsed(m_fence, m_fence->getValue(), capturedFrame);
}

void VulkanCommandBuffer::releaseResources()
{
#if VULKAN_DEBUG
    m_resources.clear();
#endif //VULKAN_DEBUG
}

bool VulkanCommandBuffer::isSafeFrame(u64 frame) const
{
    if (m_activeSwapchain)
    {
        u64 countSwapchains = m_activeSwapchain->getSwapchainImageCount();
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
    ASSERT(m_status == CommandBufferStatus::Begin, "invalid state");
    VulkanWrapper::EndCommandBuffer(m_commands);

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VulkanWrapper::CmdEndDebugUtilsLabel(m_commands);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_status = CommandBufferStatus::End;
}

void VulkanCommandBuffer::cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::array<VkClearValue, k_maxColorAttachments>& clearValues)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass area [%d, %d, %u, %u], framebuffer area [width %u, height %u]", area.offset.x, area.offset.y, area.extent.width, area.extent.height, framebuffer->getArea().m_width, framebuffer->getArea().m_height);
#endif

    VulkanCommandBuffer::captureResource(pass);
    VulkanCommandBuffer::captureResource(framebuffer);

    m_renderpassState._renderpass = pass;
    m_renderpassState._framebuffer = framebuffer;

    u32 index = 0;
    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {

        auto& image = framebuffer->getImages()[index];
        VulkanImage* vkImage = OBJECT_FROM_HANDLE(image, VulkanImage);
        VulkanCommandBuffer::captureResource(vkImage);

        VkImageLayout layout = pass->getAttachmentLayout<0>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        vkImage->setLayout(layout, VulkanImage::makeVulkanImageSubresource(vkImage, attach._layer, attach._mip));

        ++index;

#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass framebuffer image (width %u, height %u)", vkImage->getSize().width, vkImage->getSize().height);
#endif
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = pass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = static_cast<u32>(framebuffer->getImages().size());
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

    m_drawingToSwapchain = pass->isDrawingToSwapchain();
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

    u32 index = 0;
    ASSERT(pass && framebuffer, "nullptr");
    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {
        VulkanImage* image = OBJECT_FROM_HANDLE(framebuffer->getImages()[index], VulkanImage);
        ASSERT(image, "nullptr");
        VkImageLayout layout = m_renderpassState._renderpass->getAttachmentLayout<1>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        image->setLayout(layout, VulkanImage::makeVulkanImageSubresource(image, attach._layer, attach._mip));
    }

    m_isInsideRenderPass = false;
}

void VulkanCommandBuffer::cmdSetViewport(const VkViewport& viewport)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdSetViewport(m_commands, 0, 1, &viewport);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdSetScissor(const VkRect2D& scissors)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdSetScissor(m_commands, 0, 1, &scissors);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdSetStencilRef(VkStencilFaceFlags faceMask, u32 ref)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
        {
            VulkanWrapper::CmdSetStencilReference(m_commands, faceMask, ref);
        }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBeginQuery(VulkanQueryPool* pool, u32 index)
{
    //ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    //ASSERT(pool->getType() != QueryType::TimeStamp, "must be not timestamp");
    //pool->captureInsideCommandBuffer(this, 0);

    //VkQueryControlFlags flags = 0;
    //if (pool->getType() == QueryType::BinaryOcclusion)
    //{
    //    flags |= VK_QUERY_CONTROL_PRECISE_BIT;
    //}

    //[[likely]] if (m_level == CommandBufferLevel::PrimaryBuffer)
    //{
    //    VulkanWrapper::CmdBeginQuery(m_commands, pool->getHandle(), index, flags);
    //}
    //else
    //{
    //    ASSERT(false, "not implemented");
    //}

}

void VulkanCommandBuffer::cmdEndQuery(VulkanQueryPool* pool, u32 index)
{
    /*ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(pool->getType() != QueryType::TimeStamp, "must be not timestamp");
    pool->captureInsideCommandBuffer(this, 0);

    [[likely]] if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdEndQuery(m_commands, pool->getHandle(), index);
    }
    else
    {
        ASSERT(false, "not implemented");
    }*/
}

void VulkanCommandBuffer::cmdWriteTimestamp(VulkanQueryPool* pool, u32 index, VkPipelineStageFlagBits pipelineStage)
{
    /*ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(pool->getType() == QueryType::TimeStamp, "must be timestamp");
    pool->captureInsideCommandBuffer(this, 0);

    [[likely]] if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdWriteTimestamp(m_commands, pipelineStage, pool->getHandle(), index);
    }
    else
    {
        ASSERT(false, "not implemented");
    }*/
}

void VulkanCommandBuffer::cmdResetQueryPool(VulkanQueryPool* pool)
{
    /*ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!m_isInsideRenderPass, "must be out of renderpass");
    pool->captureInsideCommandBuffer(this, 0);

    VulkanWrapper::CmdResetQueryPool(m_commands, pool->getHandle(), 0, pool->getCount());*/
}

void VulkanCommandBuffer::cmdBindVertexBuffers(u32 firstBinding, u32 countBinding, const std::vector<BufferHandle>& buffers, const std::vector<u64>& offests, const std::vector<u64>& strides)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(countBinding < k_maxVertexInputBindings, "range out");

    std::array<VkBuffer, k_maxVertexInputBindings> vkBuffers;
    std::array<u64, k_maxVertexInputBindings> vkSizes;
    for (u32 index = 0; index < buffers.size(); ++index)
    {
        VulkanBuffer* vkBuffer = OBJECT_FROM_HANDLE(buffers[index], VulkanBuffer);
        vkBuffers[index] = vkBuffer->getHandle();
        vkSizes[index] = vkBuffer->getSize();
        VulkanCommandBuffer::captureResource(vkBuffer);
    }

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        static_assert(sizeof(VkDeviceSize) == sizeof(u64));
        VulkanWrapper::CmdBindVertexBuffers2(m_commands, firstBinding, countBinding, vkBuffers.data(), offests.data(), vkSizes.data(), strides.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindIndexBuffers(VulkanBuffer* buffer, VkDeviceSize offest, VkIndexType type)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(buffer);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBindIndexBuffer(m_commands, buffer->getHandle(), offest, type);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindPipeline(VulkanGraphicPipeline* pipeline)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(pipeline);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBindPipeline(m_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindPipeline(VulkanComputePipeline* pipeline)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(pipeline);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBindPipeline(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(pipeline);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle(), 0, static_cast<u32>(sets.size()), sets.data(), static_cast<u32>(offsets.size()), offsets.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanComputePipeline* pipeline, const std::vector<VkDescriptorSet>& sets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(pipeline);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getPipelineLayoutHandle(), 0, static_cast<u32>(sets.size()), sets.data(), 0, nullptr);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(isInsideRenderPass(), "inside render pass");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdDraw(m_commands, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(isInsideRenderPass(), "inside render pass");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdDrawIndexed(m_commands, indexCount, instanceCount, firstIndex, vertexOffest, firstInstance);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearAttachments(const std::vector<VkClearAttachment>& attachments, const std::vector<VkClearRect>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(isInsideRenderPass(), "inside render pass");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdClearAttachments(m_commands, static_cast<u32>(attachments.size()), attachments.data(), static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT((image->getImageAspectFlags() & VK_IMAGE_ASPECT_COLOR_BIT) == VK_IMAGE_ASPECT_COLOR_BIT, " image is not VK_IMAGE_ASPECT_COLOR_BIT");
    VulkanCommandBuffer::captureResource(image);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
        VulkanWrapper::CmdClearColorImage(m_commands, image->getHandle(), imageLayout, pColor, 1, &imageSubresourceRange);
    }
    else
    {
        ASSERT(false, "not implemented");
    }

    if (image->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        m_drawingToSwapchain = true;
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT(image->getImageAspectFlags() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT), " image is not VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT");
    VulkanCommandBuffer::captureResource(image);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
        VulkanWrapper::CmdClearDepthStencilImage(m_commands, image->getHandle(), imageLayout, pDepthStencil, 1, &imageSubresourceRange);
    }
    else
    {
        ASSERT(false, "not implemented");
    }

    ASSERT(image->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "cant be swapchain");
}

void VulkanCommandBuffer::cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "should be outside render pass");
    ASSERT(src->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "should be > 1");
    ASSERT(dst->getSampleCount() == VK_SAMPLE_COUNT_1_BIT, "should be 1");
    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdResolveImage(m_commands, src->getHandle(), srcLayout, dst->getHandle(), dstLayout, static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBlitImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageBlit>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "should be outside render pass");
    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdBlitImage(m_commands, src->getHandle(), srcLayout, dst->getHandle(), dstLayout, static_cast<u32>(regions.size()), regions.data(), VK_FILTER_LINEAR);

        for (const VkImageBlit& blit : regions)
        {
            ASSERT(blit.dstSubresource.layerCount == 1, "TODO");
            dst->setLayout(dstLayout, VulkanImage::makeVulkanImageSubresource(dst, k_generalLayer, blit.dstSubresource.mipLevel));
        }
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    VulkanCommandBuffer::captureResource(src);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdUpdateBuffer(m_commands, src->getHandle(), offset, size, data);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdCopyBufferToImage(m_commands, src->getHandle(), dst->getHandle(), layout, static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdCopyBuffer(m_commands, src->getHandle(), dst->getHandle(), static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout)
{
    VulkanCommandBuffer::cmdPipelineBarrier(image, srcStageMask, dstStageMask, layout, VulkanImage::makeVulkanImageSubresource(image));
}

void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, const RenderTexture::Subresource& resource)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VkImageLayout oldLayout = image->getLayout(resource);
    if (oldLayout == layout)
    {
        return;
    }
    VulkanCommandBuffer::captureResource(image);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        auto accessMasks = VulkanTransitionState::getAccessFlagsFromImageLayout(oldLayout, layout);
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.srcAccessMask = std::get<0>(accessMasks);
        imageMemoryBarrier.dstAccessMask = std::get<1>(accessMasks);
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = layout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image->getHandle();
        imageMemoryBarrier.subresourceRange = VulkanImage::makeImageSubresourceRange(image, resource);

        VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        const_cast<VulkanImage*>(image)->setLayout(layout, resource);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VulkanBuffer* buffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    VulkanCommandBuffer::captureResource(buffer);

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VkBufferMemoryBarrier bufferMemoryBarrier = {};
        //TODO:

        VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdDispatch(const math::Dimension3D& groups)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "must be outside");

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        VulkanWrapper::CmdDispatch(m_commands, groups.m_width, groups.m_height, groups.m_depth);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
