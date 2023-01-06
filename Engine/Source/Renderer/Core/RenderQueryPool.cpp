#include "RenderQueryPool.h"

namespace v3d
{
namespace renderer
{

RenderQueryPool::RenderQueryPool(QueryType type, u32 poolSize) noexcept
    : m_type(type)
    , m_size(poolSize)
    , m_freeIndex(0)
{
    m_queries.resize(m_size, nullptr);
    for (u32 index = 0; index < m_size; ++index)
    {
        m_queries[index] = new RenderQuery(this, index);
        m_queries[index]->_used = false;
    }
}

RenderQueryPool::~RenderQueryPool()
{
    for (u32 index = 0; index < m_size; ++index)
    {
        ASSERT(!m_queries[index]->_used, "must be false");
        delete m_queries[index];
    }
    m_queries.clear();
}

void RenderQueryPool::reset()
{
    m_freeIndex = 0;
    for (u32 index = 0; index < m_size; ++index)
    {
        m_queries[index]->_used = false;
    }
}

QueryType RenderQueryPool::getQueryType() const
{
    return m_type;
}

u32 RenderQueryPool::getPoolSize() const
{
    return m_size;
}

u32 RenderQueryPool::getSize() const
{
    return m_freeIndex;
}

bool RenderQueryPool::isFilled() const
{
    return m_freeIndex >= m_size;
}

RenderQuery* RenderQueryPool::getRenderQuery(u32 index) const
{
    ASSERT(index < m_queries.size(), "range out");
    return m_queries[index];
}

std::pair<RenderQuery*, u32> RenderQueryPool::takeFreeRenderQuery()
{
    u32 currentIndex = m_freeIndex;
    if (currentIndex < m_queries.size())
    {
        ASSERT(!m_queries[currentIndex]->_used, "already used");
        ++m_freeIndex;
        return std::pair<RenderQuery*, u32>(m_queries[currentIndex], currentIndex);
    }

    ASSERT(false, "filled");
    return std::pair<RenderQuery*, u32>(nullptr, currentIndex);
}

} //namespace renderer
} //namespace v3d
