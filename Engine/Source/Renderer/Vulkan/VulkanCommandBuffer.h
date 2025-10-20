#pragma once

#include "Common.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanResource.h"
#   include "VulkanBuffer.h"
#   include "VulkanImage.h"
#   include "VulkanRenderpass.h"
#   include "VulkanFramebuffer.h"
#   include "VulkanFence.h"
#   include "VulkanGraphicPipeline.h"
#   include "VulkanComputePipeline.h"
#   include "VulkanCommandBufferManager.h"
#   include "VulkanDevice.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanSemaphore;
    class VulkanQueryPool;
    class VulkanSwapchain;
    class VulkanCmdList;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
        VulkanResourceStateTracker& getResourceStateTracker();

        void addSemaphore(VkPipelineStageFlags mask, VulkanSemaphore* semaphore);
        void addSemaphores(VkPipelineStageFlags mask, const std::vector<VulkanSemaphore*>& semaphores);
        bool waitCompletion(u64 time = 0);

        void captureResource(VulkanResource* resource, u64 frame = 0);
        VulkanSwapchain* getActiveSwapchain() const;

        void beginCommandBuffer();
        void endCommandBuffer();

        void cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, const std::vector<VkClearValue>& clearValues);
        void cmdEndRenderPass();
        void cmdBeginRendering(const RenderPassDesc& passDesc, const FramebufferDesc& framebufferDesc, const std::vector<VkClearValue>& clearValues);
        void cmdEndRendering(const RenderPassDesc& passDesc, const FramebufferDesc& framebufferDesc);
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
        void cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, u32 first, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets);
        void cmdBindDescriptorSets(VulkanComputePipeline* pipeline, u32 first, const std::vector<VkDescriptorSet>& sets);
        void cmdBindPushConstant(VulkanGraphicPipeline* pipeline, VkShaderStageFlags stageFlags, u32 size, const void* data);
        void cmdBindPushConstant(VulkanComputePipeline* pipeline, VkShaderStageFlags stageFlags, u32 size, const void* data);

        //inside renderpass
        void cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount);
        void cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest);
        void cmdClearAttachments(const std::vector<VkClearAttachment>& attachments, const std::vector<VkClearRect>& regions);

        //outside renderpass
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);
        void cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions);
        void cmdBlitImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageBlit>& regions);
        void cmdFillBuffer(VulkanBuffer* dst, u32 offset, u64 size, u32 data);
        void cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data);
        void cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions);
        void cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions);
        void cmdCopyImageToImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageCopy>& regions);

        //sync
        void cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout);
        void cmdPipelineBarrier(VulkanImage* image, const RenderTexture::Subresource& resource, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout);
        void cmdPipelineBarrier(VulkanImage* image, const RenderTexture::Subresource& resource, VkImageLayout oldLayout, VkImageLayout newLayout);
        void cmdPipelineBarrier(VulkanBuffer* buffer, u32 offset, u32 size, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier);

        //compute
        void cmdDispatch(const math::Dimension3D& groups);

        //debug markers
        void setDebugMarker(const std::string& marker, const color::Color& color);
        void beginDebugMarker(const std::string& marker, const color::Color& color);
        void endDebugMarker(const std::string& marker, const color::Color& color);

    private:

        VulkanCommandBuffer() = delete;
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

        void init(Device::DeviceMask queueMask, VkCommandPool pool, VkCommandBuffer buffer);
        void refreshFenceStatus();
        void resetStatus();

        bool isSafeFrame(u64 frame) const;

        friend VulkanCommandBufferManager;
        friend VulkanCmdList;

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
            VulkanSwapchain*         _activeSwapchain;
        };
        RenderPassState                   m_renderpassState;
        VulkanResourceStateTracker        m_resourceStates;

        bool                              m_isInsideRenderPass;

#if VULKAN_DEBUG_MARKERS
        std::string                       m_debugName;
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    std::unordered_set<VulkanResource*>   m_resources;
#endif

#if TRACE_PROFILER_GPU_ENABLE
    tracy::VkCtx*                         m_tracyContext = nullptr;
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

    inline VulkanSwapchain* VulkanCommandBuffer::getActiveSwapchain() const
    {
        return m_renderpassState._activeSwapchain;
    }

    inline void VulkanCommandBuffer::captureResource(VulkanResource* resource, u64 frame)
    {
#if VULKAN_DEBUG
        m_resources.insert(resource);
#endif //VULKAN_DEBUG

        u64 capturedFrame = (frame == 0) ? m_capturedFrameIndex : frame;
        resource->markUsed(m_fence, m_fence->getValue(), capturedFrame);
    }

    inline VulkanResourceStateTracker& VulkanCommandBuffer::getResourceStateTracker()
    {
        return m_resourceStates;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
#include "VulkanCommandBuffer.inl"