#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCommandBuffer : public utils::NonCopyable
    {
    public:

        enum class CommandBufferType
        {
            PrimaryBuffer,
            SecondaryBuffer
        };

        enum class CommandBufferStatus
        {
            Invalid,
            CreatedBuffer,
            BeginBuffer,
            EndBuffer,
            ExecuteBuffer,
            SubmittedBuffer,
        };

        VulkanCommandBuffer(VkDevice device, VkCommandPool pool);
        ~VulkanCommandBuffer();

        VkCommandBuffer getHandle() const;

        CommandBufferStatus getStatus() const;

        void addSemaphore(VkPipelineStageFlags mask, VkSemaphore semaphore);

        void beginCommandBuffer();
        void endCommandBuffer();


        void cmdBeginRenderpass();
        void cmdEndRenderPass();

        //inside renderpass
        void cmdDraw();
        void cmdDrawIndexed();

        //outside renderpass
        void cmdClearImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor);
        void cmdClearImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil);

        //TODO: cmd list
        void cmdCopyBufferToImage();

    private:

        friend class VulkanCommandBufferManager;

        VkDevice        m_device;
        VkCommandPool   m_pool;

        VkCommandBuffer m_command;
        VkSemaphore     m_semaphore;

        CommandBufferType m_type;
        CommandBufferStatus m_status;

        std::vector<VkSemaphore>            m_semaphores;
        std::vector<VkPipelineStageFlags>   m_stageMasks;
        VkFence                             m_fence;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
