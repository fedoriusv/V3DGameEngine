#include "VulkanQuery.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDebug.h"
#include "VulkanContext.h"
#include "VulkanQueryPool.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

#if DEBUG_OBJECT_MEMORY
    std::set<VulkanQuery*> VulkanQuery::s_objects;
#endif //DEBUG_OBJECT_MEMORY

VkQueryType VulkanQuery::convertQueryTypeToVkQuery(QueryType type)
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


VulkanQuery::VulkanQuery(QueryType type, u32 count, const QueryRespose& callback, const std::string& name) noexcept
    : Query(type, count, callback)
    , m_data(nullptr)
{
    LOG_DEBUG("VulkanQuery::VulkanQuery constructor %llx", this);

#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Query" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS

#if DEBUG_OBJECT_MEMORY
    s_objects.insert(this);
#endif //DEBUG_OBJECT_MEMORY
}

VulkanQuery::~VulkanQuery()
{
    LOG_DEBUG("VulkanQuery::~VulkanQuery destructor %llx", this);

#if DEBUG_OBJECT_MEMORY
    s_objects.erase(this);
#endif //DEBUG_OBJECT_MEMORY
    ASSERT(!m_data, "not nullptr");
}

bool VulkanQuery::create()
{
    switch (m_type)
    {
    case QueryType::Occlusion:
    case QueryType::BinaryOcclusion:
    case QueryType::TimeStamp:
        m_data = malloc(m_count * sizeof(u64)); //TODO pool ?
        break;

    default:
        ASSERT(false, "not impl");
        return false;
    }

    return true;
}

void VulkanQuery::destroy()
{

    if (m_data)
    {
        delete m_data; //TODO pool ?
        m_data = nullptr;
    }
}

void VulkanQuery::dispatch(QueryResult result) const
{
    if (m_callback) //TODO checker to delete
    {
        std::invoke(m_callback, result, static_cast<u32>(m_count * sizeof(u64)), m_data);
    }
}


inline void VulkanRenderQueryState::prepare(VulkanQueryPool* pool, u32 offset, u32 count)
{
    _pool = pool;
    _offset = offset;
    _count = count;

    _recorded.resize(count, false);
    _tags.resize(count, "");
}

inline void VulkanRenderQueryState::reset()
{
    _pool = nullptr;
    _offset = 0;
    _count = 0;

    _query = nullptr;

    _recorded.clear();
    _tags.clear();
}

bool VulkanRenderQueryState::validate() const
{
#if VULKAN_DEBUG
    auto found = std::find(_recorded.cbegin(), _recorded.cend(), false);
    [[unlikely]] if (found != _recorded.cend())
    {
        return false;
    }
#endif //VULKAN_DEBUG
    return true;
}


VulkanRenderQueryBatch::VulkanRenderQueryBatch(VulkanQueryPool* pool) noexcept
    : m_pool(pool)
    , m_freeCount(pool->getCount())
    , m_usedCount(0)

    , m_renderQueryIndex(0)
{
    LOG_DEBUG("VulkanRenderQueryBatch::VulkanRenderQueryBatch constructor %llx", this);

    m_renderQueryList.resize(pool->getCount(), 
        { 
            pool, 0, 0, nullptr, {}, {}
        });
}

VulkanRenderQueryBatch::~VulkanRenderQueryBatch()
{
    LOG_DEBUG("VulkanRenderQueryBatch::~VulkanRenderQueryBatch destructor %llx", this);
}

VulkanRenderQueryState* VulkanRenderQueryBatch::prepareRenderQuery(u32 requestedCount)
{
    VulkanRenderQueryState& renderquery = m_renderQueryList[m_renderQueryIndex++];
    renderquery.prepare(m_pool, m_usedCount, requestedCount);

    ASSERT(m_freeCount >= requestedCount, "must be greater");
    m_freeCount -= requestedCount;
    m_usedCount += requestedCount;

    return &renderquery;
}

void VulkanRenderQueryBatch::resetRenderQuery()
{
    m_freeCount = m_pool->getCount();
    m_usedCount = 0;

    m_renderQueryIndex = 0;
#if VULKAN_DEBUG
    for (auto& renderQuery : m_renderQueryList)
    {
        renderQuery.reset();
    }
#endif //VULKAN_DEBUG
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER