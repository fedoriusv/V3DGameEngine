#include "VulkanRenderQuery.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDebug.h"
#include "VulkanContext.h"

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

    //for (u32 index = 0; index < )

    //m_queries.resize(m_size);
    //u32 index = 0;
    //for (RenderQuery& query : m_queries)
    //{
    //    query._pool = this;
    //    query._index = index;

    //    ++index;
    //}

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
    //hostQueryReset must be enabled
    //ASSERT(m_pool, "mist be not nullptr");
    //VulkanWrapper::ResetQueryPool(m_device, m_pool, 0, m_size);
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
    for (int poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        m_pools[poolIndex].m_currentPool = nullptr;
    }
}

VulkanRenderQueryManager::~VulkanRenderQueryManager()
{
    for (int poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    {
        ASSERT(!m_pools[poolIndex].m_currentPool, "must be nullptr");
        ASSERT(m_pools[poolIndex].m_usedPools.empty(), "must be empty");
    }
}

RenderQuery* VulkanRenderQueryManager::acquireRenderQuery(QueryType type)
{
   /* Pools& pools = m_pools[toEnumType(type)];
    if (pools.m_currentPool)
    {
        return pools.m_currentPool;
    }

    if (!pools.m_freePools.empty())
    {
        pools.m_currentPool = pools.m_freePools.front();
        pools.m_freePools.pop_front();

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
    newPool->reset();
    pools.m_currentPool = newPool;

    return pools.m_currentPool;*/
    return nullptr;
}

//void VulkanRenderQueryManager::removeRenderQueryPool(RenderQueryPool* pool)
//{
//    pool
//}

VulkanRenderQueryManager::QueryState VulkanRenderQueryManager::findRenderQueryState(Query* query) const
{
    return QueryState();
}

bool VulkanRenderQueryManager::attachRenderQueryState(const QueryState& state)
{
    return false;
}

void VulkanRenderQueryManager::updateRenderQuery()
{
    //for (int poolIndex = toEnumType(QueryType::First); poolIndex < toEnumType(QueryType::Count); ++poolIndex)
    //{
    //    Pools& pools = m_pools[poolIndex];
    //    if (pools.m_currentPool)
    //    {
    //        VulkanRenderQueryPool* vkCurrentPool = static_cast<VulkanRenderQueryPool*>(pools.m_currentPool);
    //        if (vkCurrentPool->isCaptured())
    //        {
    //            pools.m_usedPools.push_back(vkCurrentPool);
    //            pools.m_currentPool = nullptr;
    //        }
    //    }

    //    for (auto iter = pools.m_usedPools.begin(); iter != pools.m_usedPools.end();)
    //    {
    //        VulkanRenderQueryPool* vkPool = static_cast<VulkanRenderQueryPool*>(*iter);
    //        if (!vkPool->isCaptured())
    //        {
    //            //callback with query
    //            std::vector<u32> result;
    //            result.resize(vkPool->getSize());
    //            VkResult VkResult = VulkanWrapper::GetQueryPoolResults(m_device, vkPool->getHandle(), 0, 1, vkPool->getSize() * sizeof(u32), result.data(), sizeof(u32), 0);
    //            if (VkResult == VK_SUCCESS)
    //            {
    //                iter = pools.m_usedPools.erase(iter);
    //                pools.m_freePools.push_back(vkPool);

    //                continue;
    //            }
    //        }
    //        ++iter;
    //    }
    //}
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER