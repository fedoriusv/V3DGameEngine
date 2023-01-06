#include "VulkanRenderQuery.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDebug.h"
#include "VulkanContext.h"
#include "VulkanQuery.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VkQueryType VulkanRenderQueryPool::convertQueryTypeToVkQuery(QueryType type)
{
    switch (type)
    {
    case QueryType::Occlusion:
    case QueryType::BinaryOcclusion:
        return VK_QUERY_TYPE_OCCLUSION;

    case QueryType::PipelineStaticstic:
        return VK_QUERY_TYPE_PIPELINE_STATISTICS;
        
    case QueryType::TimeStamp:
        return VK_QUERY_TYPE_TIMESTAMP;

    default:
        ASSERT(false, "unknown type");
    }

    return VK_QUERY_TYPE_OCCLUSION;
}


VulkanRenderQueryPool::VulkanRenderQueryPool(VkDevice device, QueryType type, u32 size) noexcept
    : RenderQueryPool(type, size)
    , m_device(device)
    , m_pool(VK_NULL_HANDLE)
    , m_waitComlete(false)
{
    LOG_DEBUG("VulkanRenderQueryPool::VulkanRenderQueryPool constructor %llx", this);
}

VulkanRenderQueryPool::~VulkanRenderQueryPool()
{
    LOG_DEBUG("VulkanRenderQueryPool::VulkanRenderQueryPool destructor %llx", this);
    ASSERT(m_pool, "must be nullptr");
}

bool VulkanRenderQueryPool::create()
{
    VkQueryPoolCreateInfo queryPoolCreateInfo = {};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.pNext = nullptr;
    queryPoolCreateInfo.flags = 0;
    queryPoolCreateInfo.pipelineStatistics = 0;
    queryPoolCreateInfo.queryType = VulkanRenderQueryPool::convertQueryTypeToVkQuery(m_type);
    queryPoolCreateInfo.queryCount = m_size;

    VkResult result = VulkanWrapper::CreateQueryPool(m_device, &queryPoolCreateInfo, VULKAN_ALLOCATOR, &m_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanRenderQueryPool::create vkCreateQueryPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanRenderQueryPool::destroy()
{
    if (m_pool)
    {
        VulkanWrapper::DestroyQueryPool(m_device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
}

void VulkanRenderQueryPool::reset()
{
    RenderQueryPool::reset();

    ASSERT(m_pool, "must be not nullptr");
    if (VulkanDeviceCaps::getInstance()->hostQueryReset)
    {
        VulkanWrapper::ResetQueryPool(m_device, m_pool, 0, m_size);
    }
}

VkQueryPool VulkanRenderQueryPool::getHandle() const
{
    ASSERT(m_pool, "mist be not nullptr");
    return m_pool;
}

VulkanRenderQueryManager::VulkanRenderQueryManager(VkDevice device, u32 poolSize) noexcept
    : m_device(device)
    , m_poolSize(poolSize)
{
    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        m_pools[poolIndex].m_currentPool = nullptr;
    }
}

VulkanRenderQueryManager::~VulkanRenderQueryManager()
{
    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        ASSERT(!m_pools[poolIndex].m_currentPool, "must be nullptr");
        ASSERT(m_pools[poolIndex].m_usedPools.empty(), "must be empty");
    }
}

RenderQuery* VulkanRenderQueryManager::acquireRenderQuery(QueryType type, VulkanCommandBuffer* cmdBuffer)
{
    VulkanRenderQueryPool* pool = static_cast<VulkanRenderQueryPool*>(VulkanRenderQueryManager::acquireRenderQueryPool(type, cmdBuffer));
    auto [query, index] = pool->takeFreeRenderQuery();
    ASSERT(query, "must be valid");

    return query;
}

RenderQueryPool* VulkanRenderQueryManager::acquireRenderQueryPool(QueryType type, VulkanCommandBuffer* cmdBuffer)
{
    Pools& pools = m_pools[toEnumType(type)];
    if (pools.m_currentPool)
    {
        if (pools.m_currentPool->isFilled())
        {
            pools.m_usedPools.push_back(pools.m_currentPool);
        }
        else
        {
            return pools.m_currentPool;
        }
    }

    if (!pools.m_freePools.empty())
    {
        pools.m_currentPool = pools.m_freePools.front();
        pools.m_freePools.pop_front();

        resetRenderQueryPool(pools.m_currentPool, cmdBuffer);
        return pools.m_currentPool;
    }

    RenderQueryPool* newPool = new VulkanRenderQueryPool(m_device, type, m_poolSize);
    if (!newPool->create())
    {
        ASSERT(false, "Can't create new pool");
        newPool->destroy();
        delete newPool;

        return nullptr;
    }

    resetRenderQueryPool(newPool, cmdBuffer);
    pools.m_currentPool = newPool;

    return pools.m_currentPool;
}

void VulkanRenderQueryManager::resetRenderQueryPool(RenderQueryPool* pool, VulkanCommandBuffer* cmdBuffer)
{
    pool->reset();
    if (!VulkanDeviceCaps::getInstance()->hostQueryReset)
    {
        VulkanRenderQueryPool* vkPool = static_cast<VulkanRenderQueryPool*>(pool);
        cmdBuffer->cmdResetQueryPool(vkPool);
    }
}

VulkanRenderQueryManager::QueryState VulkanRenderQueryManager::findRenderQueryState(Query* query) const
{
    auto found = m_renderStates.find(query);
    if (found != m_renderStates.end())
    {
        return found->second;
    }

    return QueryState();
}

bool VulkanRenderQueryManager::applyRenderQueryState(const QueryState& state)
{
    //VulkanQuery* vkQuery = static_cast<VulkanQuery*>(state._query);
    //vkQuery->m_status = QueryStatus::Started;
    state._renderQuery->_used = true;

    //auto found = m_renderStates.emplace(state._query, state);
    return true;
}

void VulkanRenderQueryManager::updateRenderQuery()
{
    for (u32 poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        Pools& pools = m_pools[poolIndex];
        if (pools.m_currentPool)
        {
            VulkanRenderQueryPool* vkCurrentPool = static_cast<VulkanRenderQueryPool*>(pools.m_currentPool);
            if (vkCurrentPool->isCaptured())
            {
                pools.m_usedPools.push_back(vkCurrentPool);
                pools.m_currentPool = nullptr;
            }
        }

        for (auto iter = pools.m_usedPools.begin(); iter != pools.m_usedPools.end();)
        {
            VulkanRenderQueryPool* vkPool = static_cast<VulkanRenderQueryPool*>(*iter);
            if (!vkPool->isCaptured())
            {
                std::vector<u32> results;
                results.resize(vkPool->getSize());

                VkQueryResultFlags flags = 0;
                if (vkPool->m_waitComlete)
                {
                    flags |= VK_QUERY_RESULT_WAIT_BIT;
                }

                VkResult vkResult = VulkanWrapper::GetQueryPoolResults(m_device, vkPool->getHandle(), 0, vkPool->getSize(), vkPool->getSize() * sizeof(u32), results.data(), sizeof(u32), flags);
                if (vkResult == VK_SUCCESS)
                {
                    iter = pools.m_usedPools.erase(iter);
                    pools.m_freePools.push_back(vkPool);

                    [this](VulkanRenderQueryPool* pool, std::vector<u32>& results) -> void
                    {
                        for (auto iter = m_renderStates.begin(); iter != m_renderStates.end();)
                        {
                            QueryState& state = iter->second;
                            if (state._renderQuery->_pool == pool)
                            {
                                u32 result = results[state._renderQuery->_index];
                                std::invoke(state._query->callback(), QueryResult::Success, &result);

                                iter = m_renderStates.erase(iter);
                            }

                            ++iter;
                        }
                    }(vkPool, results);

                    continue;
                }
            }
            ++iter;
        }
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER