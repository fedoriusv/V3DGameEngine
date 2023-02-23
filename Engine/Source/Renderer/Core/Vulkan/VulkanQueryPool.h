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

    class VulkanQuery;
    class VulkanQueryPoolManager;
    class VulkanRenderQueryState;
    class VulkanRenderQueryBatch;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanQueryPool class. Vulkan render side
    */
    class VulkanQueryPool final : public VulkanResource
    {
    public:

        VulkanQueryPool(VkDevice device, QueryType type, u32 count) noexcept;
        ~VulkanQueryPool();

        bool create();
        void destroy();

        VkQueryPool getHandle() const;
        QueryType getType() const;
        u32 getCount() const;

    private:

        VkDevice m_device;
        VkQueryPool m_pool;
        QueryType m_type;
        u32 m_count;

        friend VulkanQueryPoolManager;
    };

    inline QueryType VulkanQueryPool::getType() const
    {
        return m_type;
    }

    inline u32 VulkanQueryPool::getCount() const
    {
        return m_count;
    }

    inline VkQueryPool VulkanQueryPool::getHandle() const
    {
        ASSERT(m_pool, "mist be not nullptr");
        return m_pool;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanQueryPoolManager final. Vulkan render side
    */
    class VulkanQueryPoolManager final
    {
    public:

        VulkanQueryPoolManager(VkDevice device, u32 poolSize) noexcept;
        ~VulkanQueryPoolManager();

        //VulkanRenderQueryState
        VulkanRenderQueryState* acquireRenderQuery(QueryType type, u32 requestedSize);
        void clear();

        void updateRenderQueries(bool wait = false);

        void markToDelete(VulkanQuery* query);

    private:

        VulkanQueryPoolManager() = delete;
        VulkanQueryPoolManager(VulkanQueryPoolManager&) = delete;
        VulkanQueryPoolManager& operator=(const VulkanQueryPoolManager&) = delete;

        VulkanQueryPool* getQueryPool(QueryType type, u32 count);

        struct Pools
        {
            std::deque<VulkanQueryPool*> _freeQueryPools;
            std::deque<VulkanQueryPool*> _usedQueryPools;
            VulkanQueryPool* _currentQueryPool;
        } m_pools[toEnumType(QueryType::Count)];

        std::map<VulkanQueryPool*, VulkanRenderQueryBatch*> m_batchQuery;
        std::vector<VulkanQuery*> m_markedToDelete;

        VkDevice m_device;
        const u32 k_poolSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
