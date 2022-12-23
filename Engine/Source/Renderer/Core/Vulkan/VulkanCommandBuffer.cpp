#include "VulkanCommandBuffer.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanComputePipeline.h"
#include "VulkanSwapchain.h"
#include "VulkanSemaphore.h"
#include "VulkanRenderQuery.h"
#include "VulkanQuery.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBuffer::VulkanCommandBuffer(Context* context, VkDevice device, CommandBufferLevel level, VulkanCommandBuffer* primaryBuffer)
    : m_device(device)

    , m_pool(VK_NULL_HANDLE)
    , m_commands(VK_NULL_HANDLE)

    , m_level(level)

    , m_fence(VK_NULL_HANDLE)
    , m_capturedFrameIndex(-1)
    , m_isInsideRenderPass(false)

    , m_context(context)
{
    LOG_DEBUG("VulkanCommandBuffer constructor %llx", this);
    m_status = CommandBufferStatus::Invalid;

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;

        VkResult result = VulkanWrapper::CreateFence(m_device, &fenceCreateInfo, VULKAN_ALLOCATOR, &m_fence);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanCommandBuffer::VulkanCommandBuffer vkCreateFence. Error %s", ErrorString(result).c_str());
        }

        m_primaryBuffer = nullptr;
    }
    else
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

    if (m_fence != VK_NULL_HANDLE)
    {
        VulkanWrapper::DestroyFence(m_device, m_fence, VULKAN_ALLOCATOR);
        m_fence = VK_NULL_HANDLE;
    }

    m_primaryBuffer = nullptr;
    m_secondaryBuffers.clear();
}

VkCommandBuffer VulkanCommandBuffer::getHandle() const
{
    ASSERT(m_commands != VK_NULL_HANDLE, "nullptr");
    return m_commands;
}

VulkanCommandBuffer::CommandBufferStatus VulkanCommandBuffer::getStatus() const
{
    return m_status;
}

void VulkanCommandBuffer::addSemaphore(VkPipelineStageFlags mask, VulkanSemaphore* semaphore)
{
    if (m_level == CommandBufferLevel::SecondaryBuffer)
    {
        ASSERT(m_primaryBuffer, "m_primaryBuffer is nullptr");
        m_primaryBuffer->addSemaphore(mask, semaphore);
    }

    ASSERT(std::find(m_semaphores.cbegin(), m_semaphores.cend(), semaphore) == m_semaphores.cend(), "added multiply times");
    semaphore->captureInsideCommandBuffer(this, 0);
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

bool VulkanCommandBuffer::waitComplete(u64 timeout)
{
    VulkanCommandBuffer::refreshFenceStatus();
    if (m_status == CommandBufferStatus::Finished)
    {
        return true;
    }

    if (m_status == CommandBufferStatus::Submit)
    {
        timeout = (timeout == 0) ? u64(~0ULL) : timeout;
        VkResult result = VulkanWrapper::WaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout);
        if (result != VK_SUCCESS)
        {
            LOG_WARNING("VulkanCommandBuffer::waitComplete vkWaitForFences. Error %s", ErrorString(result).c_str());
            return false;
        }

        return true;
    }

    ASSERT(false, "not started");
    return false;
}

void VulkanCommandBuffer::refreshFenceStatus()
{
    if (m_status == CommandBufferStatus::Submit)
    {
        if (m_level == CommandBufferLevel::PrimaryBuffer)
        {
            VkResult result = VulkanWrapper::GetFenceStatus(m_device, m_fence);
            ASSERT(m_context, "nullptr");
            if (result == VK_SUCCESS && isSafeFrame(m_context->getCurrentFrameIndex()))
            {
                m_status = CommandBufferStatus::Finished;

                result = VulkanWrapper::ResetFences(m_device, 1, &m_fence);
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
        }
    }
}

bool VulkanCommandBuffer::isBackbufferPresented() const
{
    if (m_renderpassState._renderpass)
    {
        return m_renderpassState._renderpass->isDrawingToSwapchain();
    }

    return false;
}

void VulkanCommandBuffer::init(VkCommandPool pool, VkCommandBuffer buffer)
{
    m_pool = pool;
    m_commands = buffer;
    m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;

    //Has crash inside renderdoc
/*#if VULKAN_DEBUG_MARKERS
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_commands);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS*/
}

void VulkanCommandBuffer::captureResource(VulkanResource* resource, u64 frame)
{
    std::lock_guard lock(m_mutex);

    m_capturedFrameIndex = (frame == 0) ? m_context->getCurrentFrameIndex() : frame;
    auto iter = m_resources.insert(resource);
    if (iter.second)
    {
        resource->m_counter++;
        resource->m_cmdBuffers.push_back(this);
        ASSERT(resource->m_cmdBuffers.size() >= 1, "multibuffer capture");
    }
    resource->m_status = VulkanResource::Status::Status_Captured;
    resource->m_frame = frame;
}

void VulkanCommandBuffer::releaseResources()
{
    std::lock_guard lock(m_mutex);

    for (auto& res : m_resources)
    {
        res->m_counter--;
        res->m_cmdBuffers.erase(std::remove(res->m_cmdBuffers.begin(), res->m_cmdBuffers.end(), this));
        ASSERT(res->m_counter >= 0, "less 0");
        if (!res->m_counter)
        {
            ASSERT(res->m_cmdBuffers.empty(), "less 0");
            //res->m_cmdBuffers.clear();
            res->m_status = VulkanResource::Status::Status_Done;
            res->m_frame = 0;
        }
    }
    m_resources.clear();
}

bool VulkanCommandBuffer::isSafeFrame(u64 frame)
{
    ASSERT(m_context, "nullptr");
    static u64 countSwapchains = static_cast<VulkanContext*>(m_context)->getSwapchain()->getSwapchainImageCount();

    return (m_capturedFrameIndex + countSwapchains) < frame;
}

void VulkanCommandBuffer::beginCommandBuffer()
{
    ASSERT(m_status == CommandBufferStatus::Ready, "invalid state");

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
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
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VulkanWrapper::CmdEndDebugUtilsLabel(m_commands);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_status = CommandBufferStatus::End;
}

void VulkanCommandBuffer::cmdBeginRenderpass(const VulkanRenderPass* pass, const VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::vector<VkClearValue>& clearValues)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass area (x %d, y %d, width %u, height %u), framebuffer area (width %u, height %u)", area.offset.x, area.offset.y, area.extent.width, area.extent.height, framebuffer->getArea().width, framebuffer->getArea().height);
#endif

    pass->captureInsideCommandBuffer(this, 0);
    framebuffer->captureInsideCommandBuffer(this, 0);

    m_renderpassState._renderpass = pass;
    m_renderpassState._framebuffer = framebuffer;

    u32 index = 0;
    for (auto& image : framebuffer->getImages())
    {
        VulkanImage* vkImage = static_cast<VulkanImage*>(image);
        vkImage->captureInsideCommandBuffer(this, 0);

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
    renderPassBeginInfo.clearValueCount = static_cast<u32>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

#ifdef PLATFORM_ANDROID
#   ifdef VK_QCOM_render_pass_transform
    VkRenderPassTransformBeginInfoQCOM renderPassTransformBeginInfoQCOM = {};
    if (VulkanDeviceCaps::getInstance()->renderpassTransformQCOM && pass->isDrawingToSwapchain())
    {
        VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VulkanContext*>(m_context)->getSwapchain()->getTransformFlag();
        if (preTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            renderPassTransformBeginInfoQCOM.sType = VulkanDeviceCaps::getInstance()->fixRenderPassTransformQCOMDriverIssue ? (VkStructureType)1000282000 : (VkStructureType)VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM;
            renderPassTransformBeginInfoQCOM.pNext = nullptr;
            renderPassTransformBeginInfoQCOM.transform = preTransform;

            renderPassBeginInfo.pNext = &renderPassTransformBeginInfoQCOM;
            LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass VkRenderPassTransformBeginInfoQCOM, transform %d", preTransform);
        }
    }
#   endif //VK_QCOM_render_pass_transform
#endif //PLATFORM_ANDROID

    if (VulkanDeviceCaps::getInstance()->supportRenderpass2)
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

    if (VulkanDeviceCaps::getInstance()->supportRenderpass2)
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
    for (auto& image : framebuffer->getImages())
    {
        VulkanImage* vkImage = static_cast<VulkanImage*>(image);

        VkImageLayout layout = m_renderpassState._renderpass->getAttachmentLayout<1>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        vkImage->setLayout(layout, VulkanImage::makeVulkanImageSubresource(vkImage, attach._layer, attach._mip));

        ++index;
    }

    m_isInsideRenderPass = false;
}

bool VulkanCommandBuffer::isInsideRenderPass()
{
    return m_isInsideRenderPass;
}

void VulkanCommandBuffer::cmdSetViewport(const std::vector<VkViewport>& viewports)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdSetViewport(m_commands, 0, static_cast<u32>(viewports.size()), viewports.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdSetScissor(const std::vector<VkRect2D>& scissors)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdSetScissor(m_commands, 0, static_cast<u32>(scissors.size()), scissors.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBeginQuery(VulkanRenderQueryPool* pool, u32 index)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(pool->getQueryType() != QueryType::TimeStamp, "must be not timestamp");

    VkQueryControlFlags flags = 0;
    if (pool->getQueryType() == QueryType::BinaryOcclusion)
    {
        flags |= VK_QUERY_CONTROL_PRECISE_BIT;
    }

    VulkanWrapper::CmdBeginQuery(m_commands, pool->getHandle(), index, flags);

    pool->captureInsideCommandBuffer(this, 0);
}

void VulkanCommandBuffer::cmdEndQuery(VulkanRenderQueryPool* pool, u32 index)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(pool->getQueryType() != QueryType::TimeStamp, "must be not timestamp");

    VulkanWrapper::CmdEndQuery(m_commands, pool->getHandle(), index);

    pool->captureInsideCommandBuffer(this, 0);
}

void VulkanCommandBuffer::cmdWriteTimestamp(VulkanRenderQueryPool* pool, u32 index, VkPipelineStageFlagBits pipelineStage)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(pool->getQueryType() == QueryType::TimeStamp, "must be timestamp");

    VulkanWrapper::CmdWriteTimestamp(m_commands, pipelineStage, pool->getHandle(), index);

    pool->captureInsideCommandBuffer(this, 0);
}

void VulkanCommandBuffer::cmdResetQueryPool(VulkanRenderQueryPool* pool)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanWrapper::CmdResetQueryPool(m_commands, pool->getHandle(), 0, pool->getSize());

    pool->captureInsideCommandBuffer(this, 0);
}

void VulkanCommandBuffer::cmdBindVertexBuffers(u32 firstBinding, u32 countBindinng, const std::vector<Buffer*>& buffers, const std::vector<u64>& offests)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    std::vector<VkBuffer> vkBuffers;
    vkBuffers.reserve(buffers.size());
    for (auto& buffer : buffers)
    {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
        vkBuffers.push_back(vkBuffer->getHandle());

        vkBuffer->captureInsideCommandBuffer(this, 0);
    }

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        static_assert(sizeof(VkDeviceSize) == sizeof(u64));
        VulkanWrapper::CmdBindVertexBuffers(m_commands, firstBinding, countBindinng, vkBuffers.data(), (const VkDeviceSize*)offests.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindIndexBuffers(VulkanBuffer* buffer, VkDeviceSize offest, VkIndexType type)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    buffer->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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
    pipeline->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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
    pipeline->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdBindPipeline(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    pipeline->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle(), firstSet, countSets, sets.data(), static_cast<u32>(offsets.size()), offsets.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanComputePipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    pipeline->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getPipelineLayoutHandle(), firstSet, countSets, sets.data(), 0, nullptr);
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

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdDrawIndexed(m_commands, indexCount, instanceCount, firstIndex, vertexOffest, firstInstance);
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

    ASSERT(image->getImageAspectFlags() & VK_IMAGE_ASPECT_COLOR_BIT, " image is not VK_IMAGE_ASPECT_COLOR_BIT");

    image->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
        VulkanWrapper::CmdClearColorImage(m_commands, image->getHandle(), imageLayout, pColor, 1, &imageSubresourceRange);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage * image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT(image->getImageAspectFlags() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT), " image is not VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT");
    image->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
        VulkanWrapper::CmdClearDepthStencilImage(m_commands, image->getHandle(), imageLayout, pDepthStencil, 1, &imageSubresourceRange);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "should be outside render pass");

    ASSERT(src->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "should be > 1");
    ASSERT((dst->getSampleCount() & VK_SAMPLE_COUNT_1_BIT) == VK_SAMPLE_COUNT_1_BIT, "should be 1");

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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
    src->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdCopyBuffer(m_commands, src->getHandle(), dst->getHandle(), static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(const VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout)
{
    VulkanCommandBuffer::cmdPipelineBarrier(image, srcStageMask, dstStageMask, layout, VulkanImage::makeVulkanImageSubresource(image));
}

void VulkanCommandBuffer::cmdPipelineBarrier(const VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, const Image::Subresource& resource)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VkImageLayout oldLayout = image->getLayout(resource);
    if (oldLayout == layout)
    {
        return;
    }

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        image->captureInsideCommandBuffer(this, 0);

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
    buffer->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
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

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdDispatch(const core::Dimension3D& groups)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "must be outside");

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdDispatch(m_commands, groups.width, groups.height, groups.depth);
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
