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

        VulkanCommandBuffer(VkDevice device, VkCommandPool pool);
        ~VulkanCommandBuffer();

        VkCommandBuffer getHandle() const;

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

    private:

        VkDevice        m_device;
        VkCommandPool   m_pool;

        VkCommandBuffer m_buffer;
        VkSemaphore     m_semaphore;

        CommandBufferType m_type;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
