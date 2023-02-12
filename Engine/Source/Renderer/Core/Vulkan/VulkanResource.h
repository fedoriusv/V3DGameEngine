#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCommandBuffer;

    /**
    * @brief VulkanResource class. Vulkan Render side
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
        virtual ~VulkanResource();

        Status getStatus() const;
        bool isCaptured() const;

        bool waitComplete(u64 time = 0);

        void captureInsideCommandBuffer(VulkanCommandBuffer* buffer, u64 frame) const;

    private:

        VulkanResource(const VulkanResource&) = delete;

        std::atomic<Status>     m_status;
        std::atomic<s64>        m_counter;
        std::atomic<u64>        m_frame;
        std::vector<VulkanCommandBuffer*> m_cmdBuffers;

        friend VulkanCommandBuffer;
    };

    inline VulkanResource::Status VulkanResource::getStatus() const
    {
        return m_status;
    }
    
    inline bool VulkanResource::isCaptured() const
    {
        return m_status == VulkanResource::Status::Status_Captured;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanResourceDeleter class. Vulkan Render side
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
