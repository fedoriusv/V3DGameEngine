#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanCommandBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCommandBuffer;

    class VulkanCommandBufferManager final : public utils::NonCopyable
    {
    public:

        enum class CommandTargetType
        {
            CmdDrawBuffer,
            CmdUploadBuffer
        };

        VulkanCommandBufferManager(const struct DeviceInfo* info, VkQueue queue);
        ~VulkanCommandBufferManager();

        VulkanCommandBuffer* acquireNewCmdBuffer(VulkanCommandBuffer::CommandBufferLevel level);

        bool submit(VulkanCommandBuffer* buffer, VkSemaphore signalSemaphore = VK_NULL_HANDLE);
        void updateCommandBuffers();
        void waitCompete();

        void resetPools();

    private:

        VkDevice m_device;
        VkQueue  m_queue;
        u32      m_familyIndex;

        VkCommandPoolCreateFlags    m_poolFlag;
        std::vector<VkCommandPool>  m_commandPools;

        std::deque<VulkanCommandBuffer*> m_freeCmdBuffers[VulkanCommandBuffer::CommandBufferLevelCount];
        std::deque<VulkanCommandBuffer*> m_usedCmdBuffers;


        static VkCommandPool createCommandPool(VkDevice device, VkCommandPoolCreateFlags flag, u32 familyIndex);
        static void destoryCommandPool(VkDevice device, VkCommandPool pool);

        static VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBufferLevel level);
        static void freeCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer boffer);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
