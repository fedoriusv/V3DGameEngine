#pragma once

#include "Common.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/Buffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage;
    class VulkanBuffer;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;
    class VulkanComputePipeline;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanCommandBuffer class. Vulkan Render side
    */
    class VulkanCommandBuffer final
    {
    public:

        enum CommandBufferLevel
        {
            PrimaryBuffer = 0,
            SecondaryBuffer = 1,

            CommandBufferLevelCount
        };

        enum class CommandBufferStatus
        {
            Invalid,
            Ready,
            Begin,
            End,
            Submit,
            Finished
        };

        VulkanCommandBuffer() = delete;
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

        VulkanCommandBuffer(Context* context, VkDevice device, CommandBufferLevel type, VulkanCommandBuffer* primaryBuffer = nullptr);
        ~VulkanCommandBuffer();

        VkCommandBuffer getHandle() const;

        CommandBufferStatus getStatus() const;

        void addSemaphore(VkPipelineStageFlags mask, VkSemaphore semaphore);
        bool waitComplete(u64 timeout = 0);

        void beginCommandBuffer();
        void endCommandBuffer();


        void cmdBeginRenderpass(const VulkanRenderPass* pass, const VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::vector<VkClearValue>& clearValues);
        void cmdEndRenderPass();
        bool isInsideRenderPass();

        //dynamic states
        void cmdSetViewport(const std::vector<VkViewport>& viewports);
        void cmdSetScissor(const std::vector<VkRect2D>& scissors);

        //binds
        void cmdBindVertexBuffers(u32 firstBinding, u32 countBindinng, const std::vector<Buffer*>& buffers, const std::vector<u64>& offests);
        void cmdBindIndexBuffers(VulkanBuffer* buffer, VkDeviceSize offest, VkIndexType type);
        void cmdBindPipeline(VulkanGraphicPipeline* pipeline);
        void cmdBindPipeline(VulkanComputePipeline* pipeline);
        void cmdBindDescriptorSets(VulkanGraphicPipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets);
        void cmdBindDescriptorSets(VulkanComputePipeline* pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets);

        //inside renderpass
        void cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount);
        void cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest);

        //outside renderpass
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);
        void cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions);
        void cmdBlitImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageBlit>& regions);
        void cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data);
        void cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions);
        void cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions);

        //sync
        void cmdPipelineBarrier(const VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels);
        void cmdPipelineBarrier(const VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresource);
        void cmdPipelineBarrier(VulkanBuffer* buffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier);

        //compute
        void cmdDispatch(const core::Dimension3D& groups);

    private:

        void init(VkCommandPool pool, VkCommandBuffer buffer);

        void captureResource(VulkanResource* resource, u64 frame);
        void releaseResources();

        bool isSafeFrame(u64 frame);

        friend VulkanResource;
        friend class VulkanCommandBufferManager;

        void refreshFenceStatus();

        VkDevice        m_device;

        VkCommandPool   m_pool;
        VkCommandBuffer m_commands;

        CommandBufferLevel m_level;
        CommandBufferStatus m_status;

        std::vector<VkSemaphore>            m_semaphores;
        std::vector<VkPipelineStageFlags>   m_stageMasks;
        VkFence                             m_fence;
        u64                                 m_capturedFrameIndex;

        VulkanCommandBuffer*                m_primaryBuffer;
        std::vector<VulkanCommandBuffer*>   m_secondaryBuffers;

        bool m_isInsideRenderPass;

        std::recursive_mutex m_mutex;
        std::set<VulkanResource*> m_resources;

        struct RenderPassState
        {
            const VulkanRenderPass*  _renderpass;
            const VulkanFramebuffer* _framebuffer;
        };

        RenderPassState m_renderpassState;
        Context* const m_context;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
