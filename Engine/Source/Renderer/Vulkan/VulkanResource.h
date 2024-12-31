#pragma once

#include "Renderer/Render.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
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
        VulkanResource& operator=(const VulkanResource&) = delete;

        void markUsed(VulkanFence* fence, u64 value, u64 frame);
#if VULKAN_DEBUG_MARKERS
        virtual void fenceTracker(VulkanFence* fence, u64 value, u64 frame) {};
#endif
        mutable std::unordered_map<VulkanFence*, std::tuple<u64, u64>>  m_fanceInfo;
        mutable std::recursive_mutex                                    m_mutex;
#if VULKAN_DEBUG
        mutable s64                                                     m_refCount;
#endif //VULKAN_DEBUG

        friend VulkanCommandBuffer;
    };

    inline bool VulkanResource::isUsed() const
    {
        std::lock_guard lock(m_mutex);

        bool used = std::any_of(m_fanceInfo.begin(), m_fanceInfo.end(), [](const auto& info) -> bool
            {
                VulkanFence* fence = info.first;
                return std::get<0>(info.second) >= fence->getValue();
            });

#if VULKAN_DEBUG
        if (!used)
        {
            ASSERT(m_refCount >= 0, "");
            m_refCount = 0;
            m_fanceInfo.clear();
        }
#endif //VULKAN_DEBUG

        return used;
    }

    inline void VulkanResource::markUsed(VulkanFence* fence, u64 value, u64 frame)
    {
        std::lock_guard lock(m_mutex);

#if VULKAN_DEBUG
        ++m_refCount;
#endif //VULKAN_DEBUG
        auto inserted = m_fanceInfo.emplace(fence, std::make_tuple(value, frame));
        if (!inserted.second)
        {
            inserted.first->second = { value, frame };
        }
#if VULKAN_DEBUG_MARKERS
        fenceTracker(fence, value, frame);
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanResourceDeleter class. Vulkan Render side.
    * Multithreaded
    */
    class VulkanResourceDeleter final
    {
    public:

        VulkanResourceDeleter() = default;
        VulkanResourceDeleter(const VulkanResourceDeleter&) = delete;
        ~VulkanResourceDeleter();

        void addResourceToDelete(VulkanResource* resource, const std::function<void(VulkanResource* resource)>& deleter, bool forceDelete = false);
        void updateResourceDeleter(bool forceDelete = false);

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
