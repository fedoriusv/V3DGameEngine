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

        VulkanCommandBuffer* acquireNewCmdBuffer(CommandTargetType type);

        bool submit(VulkanCommandBuffer* buffer, std::vector<VkSemaphore>& signalSemaphores);
        void waitCompete();

        void resetPools();

    private:

        VkDevice m_device;
        VkQueue  m_queue;
        u32      m_familyIndex;

        VkCommandPoolCreateFlags m_poolFlag;

        std::vector<VkCommandPool> m_commandPools;


        VkCommandPool createCommandPool(VkCommandPoolCreateFlags flag, u32 familyIndex);
        void destoryCommandPool(VkCommandPool pool);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
