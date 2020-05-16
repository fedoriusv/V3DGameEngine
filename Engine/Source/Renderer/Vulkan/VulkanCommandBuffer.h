#pragma once

#include "Common.h"
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
    class VulkanImage;
    class VulkanBuffer;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanCommandBuffer class. Vulkan Render side
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

        VulkanCommandBuffer(VkDevice device, CommandBufferLevel type, VulkanCommandBuffer* primaryBuffer = nullptr);
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
        void cmdBindDescriptorSets(VulkanGraphicPipeline * pipeline, u32 firstSet, u32 countSets, const std::vector<VkDescriptorSet>& sets, const std::vector<u32>& offsets);

        //inside renderpass
        void cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount);
        void cmdDrawIndexed(u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount, u32 vertexOffest);

        //outside renderpass
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);
        void cmdResolveImage(VulkanImage* src, VkImageLayout srcLayout, VulkanImage* dst, VkImageLayout dstLayout, const std::vector<VkImageResolve>& regions);

        //TODO: cmd list
        void cmdUpdateBuffer(VulkanBuffer* src, u32 offset, u64 size, const void* data);
        void cmdCopyBufferToImage(VulkanBuffer* src, VulkanImage* dst, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions);
        void cmdCopyBufferToBuffer(VulkanBuffer* src, VulkanBuffer* dst, const std::vector<VkBufferCopy>& regions);

        void cmdPipelineBarrier(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout, s32 layer = -1, s32 mip = -1);
        void cmdPipelineBarrier(VulkanBuffer* buffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout);

        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier);

    private:

        void captureResource(VulkanResource* resource, u64 frame);
        void releaseResources();

        bool isSafeFrame(u64 frame);

        friend VulkanResource;
        friend class VulkanCommandBufferManager;

        void refreshFenceStatus();

        VkDevice        m_device;

        VkCommandPool   m_pool;
        VkCommandBuffer m_command;

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
        Context* m_context;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
