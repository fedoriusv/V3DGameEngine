#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"
#include "Utils/Observable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage;
    class VulkanRenderPass;
    class VulkanFramebuffer;

    class VulkanCommandBuffer final : public utils::NonCopyable, public utils::Observable
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

        VulkanCommandBuffer(VkDevice device, CommandBufferLevel type, VulkanCommandBuffer* primaryBuffer = nullptr);
        ~VulkanCommandBuffer();

        VkCommandBuffer getHandle() const;

        CommandBufferStatus getStatus() const;

        void addSemaphore(VkPipelineStageFlags mask, VkSemaphore semaphore);

        void beginCommandBuffer();
        void endCommandBuffer();


        void cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, VkRect2D area, std::vector<VkClearValue>& clearValues);
        void cmdEndRenderPass();
        bool isInsideRenderPass();

        //dynamic states
        void cmdSetViewport(const std::vector <VkViewport>& viewports);
        void cmdSetScissor(const std::vector<VkRect2D>& scissors);

        //binds
        void cmdBindVertexBuffers(const std::vector<VkBuffer*> buffers, const std::vector<VkDeviceSize*> offests);

        //inside renderpass
        void cmdDraw(u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount);
        void cmdDrawIndexed();

        //outside renderpass
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VulkanImage* image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);

        //TODO: cmd list
        void cmdCopyBufferToImage();

        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& imageMemoryBarrier);
        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkMemoryBarrier& memoryBarrier);
        void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkBufferMemoryBarrier& bufferMemoryBarrier);

    private:

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

        VulkanCommandBuffer*                m_primaryBuffer;
        std::vector<VulkanCommandBuffer*>   m_secondaryBuffers;

        bool m_isInsideRenderPass;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
