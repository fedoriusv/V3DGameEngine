#pragma once

#include "Renderer/Render.h"

#ifdef VULKAN_RENDER
#   include "VulkanFunctions.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanFence final
    {
    public:

        explicit VulkanFence(VulkanDevice* device) noexcept;
        ~VulkanFence();

        VkFence getHandle() const;
        u64 getValue() const;

    private:

        VulkanFence() = delete;
        VulkanFence& operator=(const VulkanFence&) = delete;

        void incrementValue();

        VulkanDevice&    m_device;
        VkFence          m_fence;
        std::atomic<u64> m_completed;

        friend VulkanCommandBuffer;
    };

    inline VkFence VulkanFence::getHandle() const
    {
        return m_fence;
    }

    inline u64 VulkanFence::getValue() const
    {
        return m_completed;
    }

    inline void VulkanFence::incrementValue()
    {
        m_completed.fetch_add(1, std::memory_order_relaxed);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER