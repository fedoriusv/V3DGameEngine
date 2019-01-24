#include "VulkanCommandBuffer.h"
#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
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
    ASSERT(m_command, "m_command is nullptr");
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
    if (!iter.second)
    {
        ASSERT((*(iter.first))->m_cmdBuffer == resource->m_cmdBuffer, "different buffers");
    }

    resource->m_status = VulkanResource::Status::Status_Captured;
    resource->m_frame = frame;
    resource->m_cmdBuffer = this;
}

void VulkanCommandBuffer::releaseResources()
{
    std::lock_guard lock(m_mutex);
    for (auto res : m_resources)
    {
        res->m_status = VulkanResource::Status::Status_Done;
        res->m_cmdBuffer = nullptr;
        res->m_frame = 0;
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
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
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

void VulkanCommandBuffer::cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, VkRect2D area, std::vector<VkClearValue>& clearValues)
{
    pass->captureInsideCommandBuffer(this, 0);
    framebuffer->captureInsideCommandBuffer(this, 0);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = pass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = static_cast<u32>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    VulkanWrapper::CmdBeginRenderPass(m_command, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_isInsideRenderPass = true;
}

void VulkanCommandBuffer::cmdEndRenderPass()
{
    VulkanWrapper::CmdEndRenderPass(m_command);

    m_isInsideRenderPass = false;
}

bool VulkanCommandBuffer::isInsideRenderPass()
{
    return m_isInsideRenderPass;
}

void VulkanCommandBuffer::cmdSetViewport(const std::vector<VkViewport>& viewports)
{
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
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdSetScissor(m_command, 0, static_cast<u32>(scissors.size()), scissors.data());
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdBindVertexBuffers(const std::vector<VkBuffer*> buffers, const std::vector<VkDeviceSize*> offests)
{
}

void VulkanCommandBuffer::cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdDraw(m_command, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue * pColor)
{
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

void VulkanCommandBuffer::cmdUpdateBuffer(VulkanBuffer * src, u32 offset, u64 size, void * data)
{
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

void VulkanCommandBuffer::cmdCopyBufferToImage()
{
    ASSERT(!isInsideRenderPass(), "outside render pass");
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        //VulkanWrapper::CmdCopyBufferToImage(m_command, , , )
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const VkBufferCopy& region)
{
    ASSERT(!isInsideRenderPass(), "outside render pass");

    src->captureInsideCommandBuffer(this, 0);
    dst->captureInsideCommandBuffer(this, 0);
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdCopyBuffer(m_command, src->getHandle(), dst->getHandle(), 1, &region);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& imageMemoryBarrier)
{
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier)
{
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

void VulkanCommandBuffer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkBufferMemoryBarrier& bufferMemoryBarrier)
{
    if (m_level == CommandBufferLevel::PrimaryBuffer)
    {
        VulkanWrapper::CmdPipelineBarrier(m_command, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
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
