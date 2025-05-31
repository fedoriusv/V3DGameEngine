#include "VulkanCommandBuffer.h"
#ifdef VULKAN_RENDER

namespace v3d
{
namespace renderer
{
namespace vk
{

inline void VulkanCommandBuffer::cmdSetViewport(const VkViewport& viewport)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanWrapper::CmdSetViewport(m_commands, 0, 1, &viewport);
}

inline void VulkanCommandBuffer::cmdSetScissor(const VkRect2D& scissors)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanWrapper::CmdSetScissor(m_commands, 0, 1, &scissors);
}

inline void VulkanCommandBuffer::cmdSetStencilRef(VkStencilFaceFlags faceMask, u32 ref)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanWrapper::CmdSetStencilReference(m_commands, faceMask, ref);
}

inline void VulkanCommandBuffer::cmdBindVertexBuffers(u32 firstBinding, u32 countBinding, const std::vector<BufferHandle>& buffers, const std::vector<u64>& offests, const std::vector<u64>& strides)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(countBinding < k_maxVertexInputBindings, "range out");

    std::array<VkBuffer, k_maxVertexInputBindings> vkBuffers;
    std::array<u64, k_maxVertexInputBindings> vkSizes;
    {
        for (u32 index = 0; index < buffers.size(); ++index)
        {
            VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(objectFromHandle<RenderBuffer>(buffers[index]));
            vkBuffers[index] = vkBuffer->getHandle();
            vkSizes[index] = vkBuffer->getSize();
            VulkanCommandBuffer::captureResource(vkBuffer);
        }
    }

    static_assert(sizeof(VkDeviceSize) == sizeof(u64));
    if (VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE))
    {
        VulkanWrapper::CmdBindVertexBuffers2(m_commands, firstBinding, countBinding, vkBuffers.data(), offests.data(), vkSizes.data(), strides.data());
    }
    else
    {
        VulkanWrapper::CmdBindVertexBuffers(m_commands, firstBinding, countBinding, vkBuffers.data(), offests.data());
    }
}

inline void VulkanCommandBuffer::cmdBindIndexBuffers(VulkanBuffer* buffer, VkDeviceSize offest, VkIndexType type)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(buffer);
    VulkanWrapper::CmdBindIndexBuffer(m_commands, buffer->getHandle(), offest, type);
}

inline void VulkanCommandBuffer::cmdBindPipeline(VulkanGraphicPipeline* pipeline)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdBindPipeline(m_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
}

inline void VulkanCommandBuffer::cmdBindPipeline(VulkanComputePipeline* pipeline)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdBindPipeline(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
}

inline void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle(), 0, static_cast<u32>(sets.size()), sets.data(), static_cast<u32>(offsets.size()), offsets.data());
}

inline void VulkanCommandBuffer::cmdBindDescriptorSets(VulkanComputePipeline* pipeline, const std::vector<VkDescriptorSet>& sets)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdBindDescriptorSets(m_commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getPipelineLayoutHandle(), 0, static_cast<u32>(sets.size()), sets.data(), 0, nullptr);
}

inline void VulkanCommandBuffer::cmdBindPushConstant(VulkanGraphicPipeline* pipeline, VkShaderStageFlags stageFlags, u32 size, const void* data)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdPushConstants(m_commands, pipeline->getPipelineLayoutHandle(), stageFlags, 0, size, data);
}

inline void VulkanCommandBuffer::cmdBindPushConstant(VulkanComputePipeline* pipeline, VkShaderStageFlags stageFlags, u32 size, const void* data)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    VulkanCommandBuffer::captureResource(pipeline);
    VulkanWrapper::CmdPushConstants(m_commands, pipeline->getPipelineLayoutHandle(), stageFlags, 0, size, data);
}

inline void VulkanCommandBuffer::cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");

    ASSERT(isInsideRenderPass(), "inside render pass");
    VulkanWrapper::CmdDraw(m_commands, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void VulkanCommandBuffer::cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(isInsideRenderPass(), "inside render pass");

    VulkanWrapper::CmdDrawIndexed(m_commands, indexCount, instanceCount, firstIndex, vertexOffest, firstInstance);
}

inline void VulkanCommandBuffer::cmdClearAttachments(const std::vector<VkClearAttachment>& attachments, const std::vector<VkClearRect>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(isInsideRenderPass(), "inside render pass");

    VulkanWrapper::CmdClearAttachments(m_commands, static_cast<u32>(attachments.size()), attachments.data(), static_cast<u32>(regions.size()), regions.data());
}

inline void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT((image->getImageAspectFlags() & VK_IMAGE_ASPECT_COLOR_BIT) == VK_IMAGE_ASPECT_COLOR_BIT, " image is not VK_IMAGE_ASPECT_COLOR_BIT");

    VulkanCommandBuffer::captureResource(image);
    VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
    VulkanWrapper::CmdClearColorImage(m_commands, image->getHandle(), imageLayout, pColor, 1, &imageSubresourceRange);

    if (image->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        m_renderpassState._activeSwapchain = VulkanImage::getSwapchainFromImage(image);
    }
}

inline void VulkanCommandBuffer::cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");
    ASSERT(image->getImageAspectFlags() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT), " image is not VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT");

    VulkanCommandBuffer::captureResource(image);
    VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRange(image, VulkanImage::makeVulkanImageSubresource(image));
    VulkanWrapper::CmdClearDepthStencilImage(m_commands, image->getHandle(), imageLayout, pDepthStencil, 1, &imageSubresourceRange);
}

inline void VulkanCommandBuffer::cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "should be outside render pass");
    ASSERT(src->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "should be > 1");
    ASSERT(dst->getSampleCount() == VK_SAMPLE_COUNT_1_BIT, "should be 1");

    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);
    VulkanWrapper::CmdResolveImage(m_commands, src->getHandle(), srcLayout, dst->getHandle(), dstLayout, static_cast<u32>(regions.size()), regions.data());
}

inline void VulkanCommandBuffer::cmdBlitImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageBlit>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "should be outside render pass");

    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);
    VulkanWrapper::CmdBlitImage(m_commands, src->getHandle(), srcLayout, dst->getHandle(), dstLayout, static_cast<u32>(regions.size()), regions.data(), VK_FILTER_LINEAR);

    for (const VkImageBlit& blit : regions)
    {
        ASSERT(blit.dstSubresource.layerCount == 1, "TODO");
        m_resourceStates.setLayout(dst, dstLayout, VulkanImage::makeVulkanImageSubresource(dst, k_generalLayer, blit.dstSubresource.mipLevel));
    }
}

inline void VulkanCommandBuffer::cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");

    VulkanCommandBuffer::captureResource(src);
    VulkanWrapper::CmdUpdateBuffer(m_commands, src->getHandle(), offset, size, data);
}

inline void VulkanCommandBuffer::cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");

    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);
    VulkanWrapper::CmdCopyBufferToImage(m_commands, src->getHandle(), dst->getHandle(), layout, static_cast<u32>(regions.size()), regions.data());
}

inline void VulkanCommandBuffer::cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "outside render pass");

    VulkanCommandBuffer::captureResource(src);
    VulkanCommandBuffer::captureResource(dst);
    VulkanWrapper::CmdCopyBuffer(m_commands, src->getHandle(), dst->getHandle(), static_cast<u32>(regions.size()), regions.data());
}

inline void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout)
{
    VulkanCommandBuffer::cmdPipelineBarrier(image, VulkanImage::makeVulkanImageSubresource(image), srcStageMask, dstStageMask, layout);
}

inline void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage* image, const RenderTexture::Subresource& resource, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VkImageLayout oldLayout = m_resourceStates.getLayout(image, resource);
    if (oldLayout == layout)
    {
        return;
    }
    VulkanCommandBuffer::captureResource(image);

    auto [srcAccessMasks, dstAccessMasks] = VulkanTransitionState::getAccessFlagsFromImageLayout(oldLayout, layout);
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.srcAccessMask = srcAccessMasks;
    imageMemoryBarrier.dstAccessMask = dstAccessMasks;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = layout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image->getHandle();
    imageMemoryBarrier.subresourceRange = VulkanImage::makeImageSubresourceRange(image, resource);

    VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    m_resourceStates.setLayout(image, layout, resource);
}

inline void VulkanCommandBuffer::cmdPipelineBarrier(VulkanImage* image, const RenderTexture::Subresource& resource, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VulkanCommandBuffer::captureResource(image);

    auto [srcAccessMasks, dstAccessMasks] = VulkanTransitionState::getAccessFlagsFromImageLayout(oldLayout, newLayout);
    auto [srcStage, dstStage] = VulkanTransitionState::getPipelineStageFlagsFromImageLayout(image, oldLayout, newLayout);

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.srcAccessMask = srcAccessMasks;
    imageMemoryBarrier.dstAccessMask = dstAccessMasks;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image->getHandle();
    imageMemoryBarrier.subresourceRange = VulkanImage::makeImageSubresourceRange(image, resource);

    VulkanWrapper::CmdPipelineBarrier(m_commands, srcStage, dstStage, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

inline void VulkanCommandBuffer::cmdPipelineBarrier(VulkanBuffer* buffer, u32 offset, u32 size, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VulkanCommandBuffer::captureResource(buffer);

    VkBufferMemoryBarrier bufferMemoryBarrier = {};
    bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferMemoryBarrier.pNext = nullptr;
    bufferMemoryBarrier.srcAccessMask = srcAccessMask;
    bufferMemoryBarrier.dstAccessMask = dstAccessMask;
    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.buffer = buffer->getHandle();
    bufferMemoryBarrier.offset = offset;
    bufferMemoryBarrier.size = size;

    VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
}

inline void VulkanCommandBuffer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!VulkanCommandBuffer::isInsideRenderPass(), "can't be inside render pass");

    VulkanWrapper::CmdPipelineBarrier(m_commands, srcStageMask, dstStageMask, VK_DEPENDENCY_BY_REGION_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

inline void VulkanCommandBuffer::cmdDispatch(const math::Dimension3D& groups)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!isInsideRenderPass(), "must be outside");

    VulkanWrapper::CmdDispatch(m_commands, groups._width, groups._height, groups._depth);
}

inline void VulkanCommandBuffer::cmdBeginQuery(VulkanQueryPool* pool, u32 index)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
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

inline void VulkanCommandBuffer::cmdEndQuery(VulkanQueryPool* pool, u32 index)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    /*ASSERT(pool->getType() != QueryType::TimeStamp, "must be not timestamp");
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

inline void VulkanCommandBuffer::cmdWriteTimestamp(VulkanQueryPool* pool, u32 index, VkPipelineStageFlagBits pipelineStage)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    /*ASSERT(pool->getType() == QueryType::TimeStamp, "must be timestamp");
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

inline void VulkanCommandBuffer::cmdResetQueryPool(VulkanQueryPool* pool)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(!m_isInsideRenderPass, "must be out of renderpass");
    /*pool->captureInsideCommandBuffer(this, 0);

    VulkanWrapper::CmdResetQueryPool(m_commands, pool->getHandle(), 0, pool->getCount());*/
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER