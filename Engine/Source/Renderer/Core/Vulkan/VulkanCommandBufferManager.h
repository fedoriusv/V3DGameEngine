#pragma once

#include "Common.h"

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
    class VulkanSemaphore;
    class VulkanSemaphoreManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CommandTargetType enum
    */
    enum CommandTargetType
    {
        CmdDrawBuffer = 0,
        CmdUploadBuffer = 1,
        CmdTransitionBuffer = 2,

        CommandTarget_Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanCommandBufferManager class. Vulkan Render side
    */
    class VulkanCommandBufferManager final
    {
    public:
        
        VulkanCommandBufferManager() = delete;
        VulkanCommandBufferManager(const VulkanCommandBufferManager&) = delete;
        VulkanCommandBufferManager& operator=(const VulkanCommandBufferManager&) = delete;

        VulkanCommandBufferManager(Context* const context, const struct DeviceInfo* info, VulkanSemaphoreManager* const semaphoreManager, VkQueue queue);
        ~VulkanCommandBufferManager();

        VulkanCommandBuffer* acquireNewCmdBuffer(VulkanCommandBuffer::CommandBufferLevel level);

        bool submit(VulkanCommandBuffer* buffer, std::vector<VulkanSemaphore*>& signalSemaphores);
        void updateCommandBuffers();
        void waitCompete();

        void resetPools();

        static void drawToSwapchain(VulkanCommandBuffer* buffer, const class VulkanImage* swapchainImage);

    private:

        VkDevice m_device;
        VkQueue  m_queue;
        u32      m_familyIndex;

        Context* const m_context;
        VulkanSemaphoreManager* const m_semaphoreManager;

        VkCommandPoolCreateFlags    m_poolFlag;
        std::vector<VkCommandPool>  m_commandPools;

        std::deque<VulkanCommandBuffer*> m_freeCmdBuffers[VulkanCommandBuffer::CommandBufferLevelCount];
        std::deque<VulkanCommandBuffer*> m_usedCmdBuffers;

        std::vector<VkSemaphore> m_waitSemaphores;
        std::vector<VkSemaphore> m_signalSemaphores;

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
