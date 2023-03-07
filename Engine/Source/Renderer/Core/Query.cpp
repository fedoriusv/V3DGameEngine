#include "Query.h"

namespace v3d
{
namespace renderer
{

Query::Query(QueryType type, u32 count, const QueryRespose& callback) noexcept
    : m_type(type)
    , m_count(count)
    , m_callback(callback)
{
}

Query::~Query()
{
}

void Query::notifySelf(Object* caller)
{
    m_callback = nullptr;
}

} //namespace renderer
} //namespace v3d
