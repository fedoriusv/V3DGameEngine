#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanSemaphore class. Render Vulkan side
    */
    class VulkanSemaphore : public VulkanResource
    {
    public:

        enum SemaphoreStatus
        {
            Free,
            AssignToWaiting,
            AssignToSignal,
            Signaled,
        };

        VulkanSemaphore() noexcept;
        ~VulkanSemaphore();

        VkSemaphore getHandle() const;

    private:

        VulkanSemaphore(VulkanSemaphore&) = delete;
        VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

        friend class VulkanSemaphoreManager;

        VkSemaphore m_semaphore;
        SemaphoreStatus m_semaphoreStatus;
#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanSemaphoreManager class
    */
    class VulkanSemaphoreManager final
    {
    public:

        explicit VulkanSemaphoreManager(VkDevice device) noexcept;
        ~VulkanSemaphoreManager();

        VulkanSemaphore* acquireSemaphore();

        void clear();
        void updateSemaphores();

        bool markSemaphore(VulkanSemaphore* semaphore, VulkanSemaphore::SemaphoreStatus status);

        VulkanSemaphore* createSemaphore([[maybe_unused]] const std::string& name = "");
        void deleteSemaphore(VulkanSemaphore* sem);

    private:

        VulkanSemaphoreManager() = delete;
        VulkanSemaphoreManager(const VulkanSemaphoreManager&) = delete;
        VulkanSemaphoreManager& operator=(const VulkanSemaphoreManager&) = delete;

        VkDevice  m_device;

        std::deque<VulkanSemaphore*> m_freePools;
        std::deque<VulkanSemaphore*> m_usedPools;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
