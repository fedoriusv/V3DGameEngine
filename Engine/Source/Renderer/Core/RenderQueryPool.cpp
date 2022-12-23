#include "RenderQueryPool.h"

namespace v3d
{
namespace renderer
{

RenderQueryPool::RenderQueryPool(QueryType type, u32 size) noexcept
    : m_type(type)
    , m_size(size)
{
}

RenderQueryPool::~RenderQueryPool()
{
    ASSERT(m_queries.empty(), "must be empty");
}

QueryType RenderQueryPool::getQueryType() const
{
    return m_type;
}

u32 RenderQueryPool::getSize() const
{
    return m_size;
}

RenderQuery* RenderQueryPool::getRenderQuery(u32 index) const
{
    ASSERT(index < m_queries.size(), "range out");
    return m_queries[index];
}

std::pair<RenderQuery*, u32> RenderQueryPool::takeFreeRenderQuery()
{
    return std::pair<RenderQuery*, u32>(m_queries[m_freeIndex], m_freeIndex++);
}

} //namespace renderer
} //namespace v3d
