#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanResource class. Vulkan Render side
    */
    class VulkanResource
    {
    public:

        enum Status : u32
        {
            Status_Free,
            Status_Captured,
            Status_Done
        };

        VulkanResource() noexcept;
        VulkanResource(const VulkanResource&) = delete;
        virtual ~VulkanResource();

        Status getStatus() const;
        bool isCaptured() const;

        bool waitComplete(u64 time = 0);

        void captureInsideCommandBuffer(VulkanCommandBuffer* buffer, u64 frame) const;

    private:

        std::atomic<Status>     m_status;
        std::atomic<s64>        m_counter;
        std::atomic<u64>        m_frame;
        std::vector<VulkanCommandBuffer*> m_cmdBuffers;

        friend VulkanCommandBuffer;
    };

    /**
    * VulkanResourceDeleter class. Vulkan Render side
    */
    class VulkanResourceDeleter final
    {
    public:

        VulkanResourceDeleter() = default;
        VulkanResourceDeleter(const VulkanResourceDeleter&) = delete;
        ~VulkanResourceDeleter();

        void addResourceToDelete(VulkanResource* resource, const std::function<void(VulkanResource* resource)>& deleter, bool forceDelete = false);
        void updateResourceDeleter(bool wait = false);

    private:

        void resourceGarbageCollect();

        std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> m_delayedList;
        std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> m_deleterList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
