#include "Query.h"

namespace v3d
{
namespace renderer
{

Query::Query(QueryType type, const QueryRespose& callback, const std::string& name) noexcept
    : m_type(type)
    , m_callback(callback)
    , m_name(name)
{
}

Query::~Query()
{
}

QueryType Query::getType() const
{
    return m_type;
}

const std::string& Query::getName() const
{
    return m_name;
}

Query::QueryRespose Query::callback() const
{
    return m_callback;
}

} //namespace renderer
} //namespace v3d
