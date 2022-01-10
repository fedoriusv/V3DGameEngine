#include "Query.h"

namespace v3d
{
namespace renderer
{

Query::Query() noexcept
    : m_pool(nullptr)
    , m_index(~1)
{
}


QueryPool::QueryPool(QueryType type, u32 size) noexcept
    : m_type(type)
    , m_size(size)
{
    m_queries.reserve(size);
}

QueryPool::~QueryPool()
{
    ASSERT(m_queries.empty(), "must be empty");
}


QueryPoolManager::QueryPoolManager(Context* context) noexcept
    : m_context(context)
{
}

QueryPoolManager::~QueryPoolManager()
{
}

Query* QueryPoolManager::acquireQueryPool(QueryType type)
{
    return nullptr;
}

void QueryPoolManager::removeQueryPool(QueryPool* pool)
{
}

} //namespace renderer
} //namespace v3d
