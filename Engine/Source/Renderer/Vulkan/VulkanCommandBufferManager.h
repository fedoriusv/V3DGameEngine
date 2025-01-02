#pragma once

#include "Common.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanImage;
    class VulkanCommandBuffer;
    class VulkanSemaphore;
    class VulkanSemaphoreManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CommandTargetType enum
    */
    enum class CommandTargetType
    {
        CmdDrawBuffer = 0,
        CmdUploadBuffer = 1,
        CmdTransitionBuffer = 2,
        CmdResetQuerytBuffer = 3,

        Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CommandBufferLevel enum
    */
    enum CommandBufferLevel
    {
        PrimaryBuffer = 0,
        SecondaryBuffer = 1,

        CommandBufferLevelCount
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanTransitionState class. Render side
    */
    struct VulkanTransitionState
    {
        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        static VkPipelineStageFlags selectStageFlagsByImageLayout(VkImageLayout layout);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        static void transitionImage(VulkanCommandBuffer* cmdBuffer, std::tuple<VulkanImage*, RenderTexture::Subresource>& images, VkImageLayout layout, bool toCompute = false);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanCommandBufferManager class. Vulkan Render side.
    * Singlethreaded
    */
    class VulkanCommandBufferManager final
    {
    public:
        
        explicit VulkanCommandBufferManager(VulkanDevice* device, VulkanSemaphoreManager* semaphoreManager) noexcept;
        ~VulkanCommandBufferManager();

        [[nodiscard]] VulkanCommandBuffer* acquireNewCmdBuffer(Device::DeviceMask queueMask, CommandBufferLevel level);

        bool submit(VulkanCommandBuffer* buffer, const std::vector<VulkanSemaphore*>& signalSemaphores);

        void updateStatus();

        void waitCompletion();
        void waitQueueCompletion(VkQueue queue);

        void resetPools();

    private:

        VulkanCommandBufferManager() = delete;
        VulkanCommandBufferManager(const VulkanCommandBufferManager&) = delete;
        VulkanCommandBufferManager& operator=(const VulkanCommandBufferManager&) = delete;

        VulkanDevice&                           m_device;
        VulkanSemaphoreManager* const           m_semaphoreManager;

        VkCommandPoolCreateFlags                m_poolFlag;
        std::vector<std::vector<VkCommandPool>> m_commandPools;

        std::deque<VulkanCommandBuffer*>        m_freeCmdBuffers[CommandBufferLevel::CommandBufferLevelCount];
        std::vector<VulkanCommandBuffer*>       m_usedCmdBuffers;

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
