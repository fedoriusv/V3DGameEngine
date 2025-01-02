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

    class VulkanDevice;

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
            AssignForWaiting,
            AssignForSignal,
            AssignToPresent,
            Signaled
        };

        enum class SemaphoreType
        {
            Binary,
            Timeline
        };

        VulkanSemaphore(VulkanDevice* device, SemaphoreType type) noexcept;
        ~VulkanSemaphore();

        VkSemaphore getHandle() const;

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanSemaphore*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        VulkanSemaphore(VulkanSemaphore&) = delete;
        VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

        friend class VulkanSemaphoreManager;

        VulkanDevice&                m_device;
        VkSemaphore                  m_semaphore;
        SemaphoreType                m_type;
        std::atomic<SemaphoreStatus> m_status;
#if VULKAN_DEBUG_MARKERS
        std::string                  m_debugName;

        void fenceTracker(VulkanFence* fence, u64 value, u64 frame) override;
#endif
    };

    inline VkSemaphore VulkanSemaphore::getHandle() const
    {
        ASSERT(m_semaphore, "nullptr");
        return m_semaphore;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanSemaphoreManager class.
    * Multithreaded
    */
    class VulkanSemaphoreManager final
    {
    public:

        explicit VulkanSemaphoreManager(VulkanDevice* device) noexcept;
        ~VulkanSemaphoreManager();

        [[nodiscard]] VulkanSemaphore* acquireFreeSemaphore();

        void clear();
        void updateStatus(bool forced = false);

        bool markSemaphore(VulkanSemaphore* semaphore, VulkanSemaphore::SemaphoreStatus status);

        [[nodiscard]] VulkanSemaphore* createSemaphore(VulkanSemaphore::SemaphoreType type, const std::string& name = "");
        void deleteSemaphore(VulkanSemaphore* sem);

    private:

        VulkanSemaphoreManager(const VulkanSemaphoreManager&) = delete;
        VulkanSemaphoreManager& operator=(const VulkanSemaphoreManager&) = delete;

        VulkanDevice&                 m_device;
        std::mutex                    m_mutex;

        std::deque<VulkanSemaphore*>  m_freeList;
        std::vector<VulkanSemaphore*> m_usedList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
