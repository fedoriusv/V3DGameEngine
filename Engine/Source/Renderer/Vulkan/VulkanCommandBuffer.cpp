#include "VulkanCommandBuffer.h"
#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, CommandBufferLevel level, VulkanCommandBuffer* primaryBuffer)
    : m_device(device)

    , m_pool(VK_NULL_HANDLE)
    , m_command(VK_NULL_HANDLE)

    , m_level(level)

    , m_fence(VK_NULL_HANDLE)
    , m_isInsideRenderPass(false)
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
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    LOG_DEBUG("~VulkanCommandBuffer destructor %llx", this);

    //released form pool manager
    m_command = VK_NULL_HANDLE;

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
    ASSERT(m_command != VK_NULL_HANDLE, "nullptr");
    return m_command;
}

VulkanCommandBuffer::CommandBufferStatus VulkanCommandBuffer::getStatus() const
{
    return m_status;
}

void VulkanCommandBuffer::addSemaphore(VkPipelineStageFlags mask, VkSemaphore semaphore)
{
    if (m_level == VulkanCommandBuffer::SecondaryBuffer)
    {
        ASSERT(m_primaryBuffer, "m_primaryBuffer is nullptr");
        m_primaryBuffer->addSemaphore(mask, semaphore);
    }

    m_stageMasks.push_back(mask);
    m_semaphores.push_back(semaphore);
}

bool VulkanCommandBuffer::waitComplete(u64 timeout)
{
    VulkanCommandBuffer::refreshFenceStatus();
    if (m_status == VulkanCommandBuffer::CommandBufferStatus::Finished)
    {
        return true;
    }

    if (m_status == VulkanCommandBuffer::CommandBufferStatus::Submit)
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
    if (m_status == VulkanCommandBuffer::CommandBufferStatus::Submit)
    {
        if (m_level == VulkanCommandBuffer::PrimaryBuffer)
        {
            VkResult result = VulkanWrapper::GetFenceStatus(m_device, m_fence);
            if (result == VK_SUCCESS)
            {
                m_status = VulkanCommandBuffer::CommandBufferStatus::Finished;

                result = VulkanWrapper::ResetFences(m_device, 1, &m_fence);
                if (result != VK_SUCCESS)
                {
                    m_status = VulkanCommandBuffer::CommandBufferStatus::Invalid;
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

void VulkanCommandBuffer::captureResource(VulkanResource* resource, u64 frame)
{
    std::lock_guard lock(m_mutex);

    auto iter = m_resources.insert(resource);
    if (iter.second)
    {
        resource->m_counter++;
        resource->m_cmdBuffers.push_back(this);
    }
    resource->m_status = VulkanResource::Status::Status_Captured;
    resource->m_frame = frame;
}

void VulkanCommandBuffer::releaseResources()
{
    std::lock_guard lock(m_mutex);
    for (auto res : m_resources)
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

void VulkanCommandBuffer::beginCommandBuffer()
{
    ASSERT(m_status == VulkanCommandBuffer::CommandBufferStatus::Ready, "invalid state");

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

    VkResult result = VulkanWrapper::BeginCommandBuffer(m_command, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBuffer::beginCommandBuffer vkBeginCommandBuffer. Error %s", ErrorString(result).c_str());
    }

    m_status = CommandBufferStatus::Begin;
}

void VulkanCommandBuffer::endCommandBuffer()
{
    ASSERT(m_status == VulkanCommandBuffer::CommandBufferStatus::Begin, "invalid state");
    VulkanWrapper::EndCommandBuffer(m_command);

    m_status = CommandBufferStatus::End;
}

void VulkanCommandBuffer::cmdBeginRenderpass(const VulkanRenderPass* pass, const VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::vector<VkClearValue>& clearValues)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    pass->captureInsideCommandBuffer(this, 0);
    framebuffer->captureInsideCommandBuffer(this, 0);
    for (auto& image : framebuffer->getImages())
    {
        VulkanImage* vkImage = static_cast<VulkanImage*>(image);
        vkImage->captureInsideCommandBuffer(this, 0);
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = pass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = static_cast<u32>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

	if (VulkanDeviceCaps::getInstance()->supportRenderpass2)
	{
		VkSubpassBeginInfoKHR subpassBeginInfo = {};
		subpassBeginInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR;
		subpassBeginInfo.pNext = nullptr;
		subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

		VulkanWrapper::CmdBeginRenderPass2(m_command, &renderPassBeginInfo, &subpassBeginInfo);
	}
	else
	{
		VulkanWrapper::CmdBeginRenderPass(m_command, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

    m_isInsideRenderPass = true;
}

void VulkanCommandBuffer::cmdEndRenderPass()
{
	if (VulkanDeviceCaps::getInstance()->supportRenderpass2)
	{
		VkSubpassEndInfoKHR subpassEndInfo = {};
		subpassEndInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR;
		subpassEndInfo.pNext = nullptr;

		VulkanWrapper::CmdEndRenderPass2(m_command, &subpassEndInfo);
	}
	else
	{
		VulkanWrapper::CmdEndRenderPass(m_command);
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
        VulkanWrapper::CmdSetViewport(m_command, 0, static_cast<u32>(viewports.size()), viewports.data());
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
        VulkanWrapper::CmdSetScissor(m_command, 0, static_cast<u32>(scissors.size()), scissors.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
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
        VulkanWrapper::CmdBindVertexBuffers(m_command, firstBinding, countBindinng, vkBuffers.data(), (const VkDeviceSize*)offests.data());
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
        VulkanWrapper::CmdBindIndexBuffer(m_command, buffer->getHandle(), offest, type);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindPipeline(VulkanGraphicPipeline * pipeline)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    pipeline->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdBindPipeline(m_command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanGraphicPipeline * pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    pipeline->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdBindDescriptorSets(m_command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle(), firstSet, countSets, sets.data(), static_cast<u32>(offsets.size()), offsets.data());
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
        VulkanWrapper::CmdDraw(m_command, vertexCount, instanceCount, firstVertex, firstInstance);
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
        VulkanWrapper::CmdDrawIndexed(m_command, indexCount, instanceCount, firstIndex, vertexOffest, firstInstance);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue * pColor)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");

    ASSERT(image->getImageAspectFlags() == VK_IMAGE_ASPECT_COLOR_BIT, " image is not VK_IMAGE_ASPECT_COLOR_BIT");

    image->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image);
        VulkanWrapper::CmdClearColorImage(m_command, image->getHandle(), imageLayout, pColor, 1, &imageSubresourceRange);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage * image, VkImageLayout imageLayout, const VkClearDepthStencilValue * pDepthStencil)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT(image->getImageAspectFlags() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT), " image is not VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT");

    image->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image);
        VulkanWrapper::CmdClearDepthStencilImage(m_command, image->getHandle(), imageLayout, pDepthStencil, 1, &imageSubresourceRange);
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
    ASSERT(dst->getSampleCount() == VK_SAMPLE_COUNT_1_BIT, "should be 1");

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);

    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdResolveImage(m_command, src->getHandle(), srcLayout, dst->getHandle(), dstLayout, static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdUpdateBuffer(VulkanBuffer * src, u32 offset, u64 size, void * data)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");

    src->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdUpdateBuffer(m_command, src->getHandle(), offset, size, data);
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
        VulkanWrapper::CmdCopyBufferToImage(m_command, src->getHandle(), dst->getHandle(), layout, static_cast<u32>(regions.size()), regions.data());
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
        VulkanWrapper::CmdCopyBuffer(m_command, src->getHandle(), dst->getHandle(), static_cast<u32>(regions.size()), regions.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage * image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, s32 layer, s32 mip)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    image->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        auto accessMasks = VulkanImage::getAccessFlagsFromImageLayout(image->getLayout(layer, mip), layout);

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.srcAccessMask = std::get<0>(accessMasks);
        imageMemoryBarrier.dstAccessMask = std::get<1>(accessMasks);
        imageMemoryBarrier.oldLayout = image->getLayout(layer, mip);
        imageMemoryBarrier.newLayout = layout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image->getHandle();
        imageMemoryBarrier.subresourceRange = VulkanImage::makeImageSubresourceRange(image, layer, mip);

        image->setLayout(layout, layer, mip);

        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VulkanBuffer* buffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    buffer->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VkBufferMemoryBarrier bufferMemoryBarrier = {};
        //TODO:

        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
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
        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
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
