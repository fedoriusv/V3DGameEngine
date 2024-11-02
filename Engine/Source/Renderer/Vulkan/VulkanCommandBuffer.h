#pragma once

#include "Common.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
    class Buffer;
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage;
    class VulkanBuffer;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;
    class VulkanComputePipeline;
    class VulkanSemaphore;
    class VulkanQueryPool;
    class VulkanDevice;
    class VulkanFence;
    class VulkanCmdList;
    class VulkanCommandBufferManager;
    class VulkanSwapchain;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum CommandBufferLevel
    {
        PrimaryBuffer = 0,
        SecondaryBuffer = 1,

        CommandBufferLevelCount
    };

    /**
    * @brief VulkanCommandBuffer class. Vulkan Render side
    */
    class VulkanCommandBuffer final
    {
    public:

        enum class CommandBufferStatus
        {
            Invalid,
            Ready,
            Begin,
            End,
            Submit,
            Finished
        };

        explicit VulkanCommandBuffer(VulkanDevice* device, CommandBufferLevel type, VulkanCommandBuffer* primaryBuffer = nullptr) noexcept;
        ~VulkanCommandBuffer();

        VkCommandBuffer getHandle() const;
        CommandBufferStatus getStatus() const;

        void addSemaphore(VkPipelineStageFlags mask, VulkanSemaphore* semaphore);
        void addSemaphores(VkPipelineStageFlags mask, const std::vector<VulkanSemaphore*>& semaphores);
        bool waitCompletion(u64 time = 0);

        void captureResource(VulkanResource* resource, u64 frame = 0);
        bool isBackbufferPresented() const;

        void beginCommandBuffer();
        void endCommandBuffer();

        void cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::array<VkClearValue, k_maxColorAttachments>& clearValues);
        void cmdEndRenderPass();
        bool isInsideRenderPass() const;

        //dynamic states
        void cmdSetViewport(const VkViewport& viewports);
        void cmdSetScissor(const VkRect2D& scissors);
        void cmdSetStencilRef(VkStencilFaceFlags faceMask, u32 ref);

        //query
        void cmdBeginQuery(VulkanQueryPool* pool, u32 index);
        void cmdEndQuery(VulkanQueryPool* pool, u32 index);
        void cmdWriteTimestamp(VulkanQueryPool* pool, u32 index, VkPipelineStageFlagBits pipelineStage);
        void cmdResetQueryPool(VulkanQueryPool* pool);

        //binds
        void cmdBindVertexBuffers(u32 firstBinding, u32 countBinding, const std::vector<BufferHandle>& buffers, const std::vector<u64>& offests, const std::vector<u64>& strides);
        void cmdBindIndexBuffers(VulkanBuffer* buffer, VkDeviceSize offest, VkIndexType type);
        void cmdBindPipeline(VulkanGraphicPipeline* pipeline);
        void cmdBindPipeline(VulkanComputePipeline* pipeline);
        void cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets);
        void cmdBindDescriptorSets(VulkanComputePipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets);

        //inside renderpass
        void cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount);
        void cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest);
        void cmdClearAttachments(const std::vector<VkClearAttachment>& attachments, const std::vector<VkClearRect>& regions);

        //outside renderpass
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);
        void cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions);
        void cmdBlitImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageBlit>& regions);
        void cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data);
        void cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions);
        void cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions);

        //sync
        void cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout);
        void cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, const RenderTexture::Subresource& resource);
        void cmdPipelineBarrier(VulkanBuffer* buffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier);

        //compute
        void cmdDispatch(const math::Dimension3D& groups);

    private:

        VulkanCommandBuffer() = delete;
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

        void init(Device::DeviceMask queueMask, VkCommandPool pool, VkCommandBuffer buffer);

        void releaseResources();

        bool isSafeFrame(u64 frame) const;

        friend VulkanResource;
        friend VulkanDevice;
        friend VulkanCmdList;
        friend VulkanCommandBufferManager;

        void refreshFenceStatus();

        VulkanDevice&                     m_device;
        VkCommandPool                     m_pool;
        VkCommandBuffer                   m_commands;
        CommandBufferLevel                m_level;
        CommandBufferStatus               m_status;
        u32                               m_queueIndex;

        std::vector<VulkanSemaphore*>     m_semaphores;
        std::vector<VkPipelineStageFlags> m_stageMasks;
        u64                               m_capturedFrameIndex;
        VulkanFence*                      m_fence;

        VulkanCommandBuffer*              m_primaryBuffer;
        std::vector<VulkanCommandBuffer*> m_secondaryBuffers;

        struct RenderPassState
        {
            const VulkanRenderPass*  _renderpass;
            const VulkanFramebuffer* _framebuffer;
        };
        RenderPassState                   m_renderpassState;
        VulkanSwapchain*                  m_activeSwapchain;

        bool                              m_drawingToSwapchain;
        bool                              m_isInsideRenderPass;

#if VULKAN_DEBUG_MARKERS
        std::string                       m_debugName;
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    std::unordered_set<VulkanResource*>   m_resources;
#endif
    };

    inline VulkanCommandBuffer::CommandBufferStatus VulkanCommandBuffer::getStatus() const
    {
        return m_status;
    }

    inline bool VulkanCommandBuffer::isInsideRenderPass() const
    {
        return m_isInsideRenderPass;
    }

    inline VkCommandBuffer VulkanCommandBuffer::getHandle() const
    {
        ASSERT(m_commands != VK_NULL_HANDLE, "nullptr");
        return m_commands;
    }

    inline bool VulkanCommandBuffer::isBackbufferPresented() const
    {
        return m_drawingToSwapchain;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER