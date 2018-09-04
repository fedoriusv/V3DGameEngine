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

        VulkanCommandBufferManager(VkQueue queue);
        ~VulkanCommandBufferManager();

        VulkanCommandBuffer* acquireNewCmdBuffer(CommandTargetType type);

        void submit(VulkanCommandBuffer* buffer);
        void waitCompete();

    private:

        std::vector<VkCommandPool> m_commandPools;
        VkQueue m_queue;

        bool createCommandPool();
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
