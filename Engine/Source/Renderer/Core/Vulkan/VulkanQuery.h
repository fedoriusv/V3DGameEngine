#pragma once

#include "Renderer/Core/Query.h"

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

    class VulkanContext;
    class VulkanQueryPool;
    class VulkanQueryPoolManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanQuery class. Vulkan render side
    */
    class VulkanQuery : public Query, public VulkanResource
    {
    public:

        static VkQueryType convertQueryTypeToVkQuery(QueryType type);

        VulkanQuery() = delete;
        VulkanQuery(VulkanQuery&) = delete;

        VulkanQuery(QueryType type, u32 size, const QueryRespose& callback, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanQuery();

    private:

        friend VulkanQueryPoolManager;
#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderQueryState struct. Vulkan render side
    */
    struct VulkanRenderQueryState
    {
        VulkanQueryPool* _pool;
        u32 _offset;
        u32 _count;

        const VulkanQuery* _query;

        std::vector<bool> _recorded;
        std::vector<std::string> _tags;

        void prepare(VulkanQueryPool* pool, u32 offset, u32 count);
        void reset();
        bool validate() const;
    };

    /**
    * @brief VulkanRenderQueryBatch class. Vulkan render side
    */
    class VulkanRenderQueryBatch final
    {
    public:

        explicit VulkanRenderQueryBatch(VulkanQueryPool* pool) noexcept;
        ~VulkanRenderQueryBatch();

        //VulkanRenderQueryState
        VulkanRenderQueryState* prepareRenderQuery(u32 requestedCount);
        void resetRenderQuery();

        u32 getBatchCount() const;
        const VulkanRenderQueryState& operator[](u32 index) const;

        u32 getUsedCount() const;
        u32 getFreeCount() const;

    private:

        VulkanQueryPool* const m_pool;
        u32 m_freeCount;
        u32 m_usedCount;

        u32 m_renderQueryIndex;
        std::vector<VulkanRenderQueryState> m_renderQueryList;
    };

    inline u32 VulkanRenderQueryBatch::getBatchCount() const
    {
        return m_renderQueryIndex;
    }

    inline const VulkanRenderQueryState& VulkanRenderQueryBatch::operator[](u32 index) const
    {
        return m_renderQueryList[index];
    }

    inline u32 VulkanRenderQueryBatch::getUsedCount() const
    {
        return m_usedCount;
    }

    inline u32 VulkanRenderQueryBatch::getFreeCount() const
    {
        return m_freeCount;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
