#include "Query.h"

namespace v3d
{
namespace renderer
{

Query::Query(QueryType type, QueryRespose callback) noexcept
    : m_type(type)
    , m_callback(callback)
{
}

QueryType Query::getType() const
{
    return m_type;
}

Query::QueryRespose Query::getCallback() const
{
    return m_callback;
}

} //namespace renderer
} //namespace v3d
