#pragma once

#include "Renderer/Render.h"

#ifdef VULKAN_RENDER
#   include "VulkanFence.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanFence;
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanResource class. Vulkan Render side
    */
    class VulkanResource
    {
    public:

        VulkanResource() noexcept;
        virtual ~VulkanResource();

        bool isUsed() const;

    private:

        VulkanResource(const VulkanResource&) = delete;

        void markUsed(VulkanFence* fence, u64 value);

        std::map<VulkanFence*, std::atomic<s64>> m_fanceInfo;
        std::atomic<u64>                         m_frame;
        std::recursive_mutex                     m_mutex;
#if VULKAN_DEBUG
        s64 m_ref;
#endif //VULKAN_DEBUG

        friend VulkanCommandBuffer;
    };

    inline bool VulkanResource::isUsed() const
    {
        bool used = std::any_of(m_fanceInfo.begin(), m_fanceInfo.end(), [](const auto& info) -> bool
            {
                return info.second < info.first->getValue();
            });

        return used;
    }

    inline void VulkanResource::markUsed(VulkanFence* fence, u64 value)
    {
        std::scoped_lock lock(m_mutex);

        auto inserted = m_fanceInfo.emplace(fence, value);
        if (!inserted.second)
        {
            inserted.first->second = value;
        }
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
        void updateResourceDeleter();

    private:

        void resourceGarbageCollect();

        std::recursive_mutex m_mutex;
        std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> m_delayedList;
        std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> m_deleterList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
