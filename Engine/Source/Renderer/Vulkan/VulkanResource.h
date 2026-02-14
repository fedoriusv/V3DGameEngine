#pragma once

#include "Renderer/Render.h"
#include "Thread/Spinlock.h"

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
    class VulkanImage;
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
        mutable thread::Spinlock                                        m_mutex;
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
        {
            TRACE_PROFILER_RENDER_SCOPE("markUsed", color::rgba8::BLACK);

            std::lock_guard lock(m_mutex);
#if VULKAN_DEBUG
            ++m_refCount;
#endif //VULKAN_DEBUG
            auto found = m_fanceInfo.find(fence);
            if (found != m_fanceInfo.end())
            {
                found->second = { value, frame };
            }
            else
            {
                [[maybe_unused]] auto inserted = m_fanceInfo.emplace(fence, std::make_tuple(value, frame));
                ASSERT(inserted.second, "must be unique");
            }
#if VULKAN_DEBUG_MARKERS
            fenceTracker(fence, value, frame);
#endif
        }
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
        void resourceGarbageCollect(bool forceDelete = false);

    private:

        thread::Spinlock m_mutex;
        std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> m_delayedList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanResourceStateTracker class. Vulkan Render side.
    * Multithreaded. No sync, only own thread
    */
    class VulkanResourceStateTracker
    {
    public:

        VkImageLayout getLayout(VulkanImage* image, const RenderTexture::Subresource& resource) const;
        VkImageLayout setLayout(VulkanImage* image, VkImageLayout layout, const RenderTexture::Subresource& resource);

        void finalizeGlobalState();
        void prepareGlobalState(const std::function<VulkanCommandBuffer* ()>& cmdBufferGetter);

    private:

        mutable std::unordered_map<VulkanImage*, std::tuple<std::vector<VkImageLayout>, std::vector<VkImageLayout>>> m_states;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
