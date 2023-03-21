#include "VulkanQueryPool.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDebug.h"
#include "VulkanQuery.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanQueryPool::VulkanQueryPool(VkDevice device, QueryType type, u32 count) noexcept
    : m_device(device)
    , m_pool(VK_NULL_HANDLE)
    , m_type(type)
    , m_count(count)
{
    LOG_DEBUG("VulkanQueryPool::VulkanQueryPool constructor %llx", this);
}

VulkanQueryPool::~VulkanQueryPool()
{
    LOG_DEBUG("VulkanQueryPool::VulkanQueryPool destructor %llx", this);
    ASSERT(m_pool == VK_NULL_HANDLE, "must be nullptr");
}

bool VulkanQueryPool::create()
{
    VkQueryPoolCreateInfo queryPoolCreateInfo = {};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.pNext = nullptr;
    queryPoolCreateInfo.flags = 0;
    queryPoolCreateInfo.pipelineStatistics = 0;
    queryPoolCreateInfo.queryType = VulkanQuery::convertQueryTypeToVkQuery(m_type);
    queryPoolCreateInfo.queryCount = m_count;

    VkResult result = VulkanWrapper::CreateQueryPool(m_device, &queryPoolCreateInfo, VULKAN_ALLOCATOR, &m_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanQueryPool::create vkCreateQueryPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanQueryPool::destroy()
{
    if (m_pool)
    {
        VulkanWrapper::DestroyQueryPool(m_device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
}

VulkanQueryPoolManager::VulkanQueryPoolManager(VkDevice device, u32 poolSize) noexcept
    : m_device(device)
    , k_poolSize(poolSize)
{
    LOG_DEBUG("VulkanQueryPoolManager::VulkanQueryPoolManager constructor %llx", this);

    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        m_pools[poolIndex]._currentQueryPool = nullptr;
    }
}

VulkanQueryPoolManager::~VulkanQueryPoolManager()
{
    LOG_DEBUG("VulkanQueryPoolManager::VulkanQueryPoolManager destructor %llx", this);

    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        ASSERT(!m_pools[poolIndex]._currentQueryPool, "must be nullptr");
        ASSERT(m_pools[poolIndex]._usedQueryPools.empty(), "must be empty");
        ASSERT(m_pools[poolIndex]._freeQueryPools.empty(), "must be empty");
    }
}

VulkanRenderQueryState* VulkanQueryPoolManager::acquireRenderQuery(QueryType type, u32 requestedSize)
{
    Pools& pools = m_pools[toEnumType(type)];
    if (!pools._currentQueryPool)
    {
        VulkanRenderQueryBatch* poolBatch = nullptr;
        if (pools._freeQueryPools.empty())
        {
            u32 newPoolSize = k_poolSize;
            if (requestedSize > k_poolSize)
            {
                LOG_WARNING("VulkanQueryPoolManager::acquireRenderQuery: min pool size less than reqested. memoryMinQueryPoolCount: %d, reqested %d", k_poolSize, requestedSize);
                newPoolSize = requestedSize;

            }
            pools._currentQueryPool = VulkanQueryPoolManager::getQueryPool(type, newPoolSize);
            ASSERT(pools._currentQueryPool, "nullptr");

            ASSERT(m_batchQuery.find(pools._currentQueryPool) == m_batchQuery.end(), "already present");
            auto iter = m_batchQuery.emplace(pools._currentQueryPool, new VulkanRenderQueryBatch(pools._currentQueryPool));
            poolBatch = iter.first->second;
        }
        else
        {
            pools._currentQueryPool = pools._freeQueryPools.front();
            pools._freeQueryPools.pop_front();

            auto iter = m_batchQuery.find(pools._currentQueryPool);
            ASSERT(iter != m_batchQuery.end(), "must be presented");
            poolBatch = iter->second;
            poolBatch->resetRenderQuery();
        }

        ASSERT(poolBatch, "nulllptr");
        VulkanRenderQueryState* renderQuery = poolBatch->prepareRenderQuery(requestedSize);
        return renderQuery;
    }

    auto iter = m_batchQuery.find(pools._currentQueryPool);
    ASSERT(iter != m_batchQuery.end(), "must be presented");
    VulkanRenderQueryBatch* poolBatch = iter->second;

    if (poolBatch->getFreeCount() < requestedSize)
    {
        pools._usedQueryPools.push_back(pools._currentQueryPool);
        if (!pools._freeQueryPools.empty())
        {
            pools._currentQueryPool = pools._freeQueryPools.front();
            pools._freeQueryPools.pop_front();

            auto iter = m_batchQuery.find(pools._currentQueryPool);
            ASSERT(iter != m_batchQuery.end(), "must be presented");
            poolBatch = iter->second;
        }
        else
        {
            ASSERT(requestedSize <= k_poolSize, "pool size less than reqested");
            pools._currentQueryPool = VulkanQueryPoolManager::getQueryPool(type, k_poolSize);
            ASSERT(pools._currentQueryPool, "nullptr");

            ASSERT(m_batchQuery.find(pools._currentQueryPool) == m_batchQuery.end(), "already present");
            auto iter = m_batchQuery.emplace(pools._currentQueryPool, new VulkanRenderQueryBatch(pools._currentQueryPool));
            poolBatch = iter.first->second;
        }
    }

    VulkanRenderQueryState* renderQuery = poolBatch->prepareRenderQuery(requestedSize);
    return renderQuery;
}

void VulkanQueryPoolManager::clear()
{
    for (auto batch : m_batchQuery)
    {
        delete batch.second;
    }
    m_batchQuery.clear();

    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        ASSERT(!m_pools[poolIndex]._currentQueryPool, "must be nullptr");
        ASSERT(m_pools[poolIndex]._usedQueryPools.empty(), "must be empty");

        Pools& pools = m_pools[poolIndex];
        for (VulkanQueryPool* pool : pools._freeQueryPools)
        {
            ASSERT(!pool->isCaptured(), "must be free");

            pool->destroy();
            delete pool;
        }
        pools._freeQueryPools.clear();
    }

    m_markedToDelete.clear();
}

void VulkanQueryPoolManager::updateRenderQueries(bool wait)
{
    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        Pools& pools = m_pools[poolIndex];
        if (pools._currentQueryPool && pools._currentQueryPool->isCaptured())
        {
            pools._usedQueryPools.push_back(pools._currentQueryPool);
            pools._currentQueryPool = nullptr;
        }

        ASSERT(pools._usedQueryPools.size() < 1024, "too many pools are not resolved. Enable VULKAN_DEBUG for debug");
        for (auto iter = pools._usedQueryPools.begin(); iter != pools._usedQueryPools.end();)
        {
            VulkanQueryPool* pool = (*iter);
            if (!pool->isCaptured())
            {
                VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT;
                if (wait)
                {
                    flags |= VK_QUERY_RESULT_WAIT_BIT;
                }

                auto iterBatch = m_batchQuery.find(pool);
                ASSERT(iterBatch != m_batchQuery.end(), "must be presented");

                bool isFree = false;
                VulkanRenderQueryBatch& poolBatch = *iterBatch->second;
                for (u32 index = 0; index < poolBatch.getBatchCount(); ++index)
                {
                    const VulkanRenderQueryState& renderQuery = poolBatch[index];
#if VULKAN_DEBUG
                    if (!renderQuery.validate())
                    {
                        LOG_WARNING("VulkanQueryPoolManager::updateRenderQueries: Some indices are not executed. For Query %s", renderQuery._query->m_debugName.c_str());
                    }
#endif //VULKAN_DEBUG
                    auto deletedQuery = std::find(m_markedToDelete.begin(), m_markedToDelete.end(), renderQuery._query);
                    if (deletedQuery != m_markedToDelete.end())
                    {
                        //Skip removed query
                        isFree = true;
                        continue;
                    }

                    VkResult vkResult = VulkanWrapper::GetQueryPoolResults(m_device, pool->getHandle(), renderQuery._offset, renderQuery._count, renderQuery._count * sizeof(u64), renderQuery._query->m_data, sizeof(u64), flags);
                    if (vkResult == VK_SUCCESS)
                    {
                        renderQuery._query->dispatch(QueryResult::Success);
                        isFree = true;
                    }
                    else if (vkResult == VK_NOT_READY)
                    {
                        //Pool is not ready yet. skip for the next time
                        isFree = false;
                        continue;
                    }
                    else
                    {
                        LOG_WARNING("VulkanQueryPoolManager::updateRenderQueries: vkGetQueryPoolResults result: %s", ErrorString(vkResult).c_str());
                        ASSERT(false, "vkGetQueryPoolResults: unhandled behavior");
                    }
                }

                if (isFree)
                {
                    iter = pools._usedQueryPools.erase(iter);
                    pools._freeQueryPools.push_back(pool);
                    continue;
                }
            }

            ++iter;
        }
    }
}

VulkanQueryPool* VulkanQueryPoolManager::getQueryPool(QueryType type, u32 count)
{
    VulkanQueryPool* newPool = new VulkanQueryPool(m_device, type, count);
    if (!newPool->create())
    {
        delete newPool;

        ASSERT(false, "pool doesn't create");
        return nullptr;
    }

    return newPool;
}

void VulkanQueryPoolManager::markToDelete(VulkanQuery* query)
{
    m_markedToDelete.push_back(query);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER