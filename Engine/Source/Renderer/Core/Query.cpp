#include "Query.h"

namespace v3d
{
namespace renderer
{

Query::Query(QueryType type, u32 size, const QueryRespose& callback, const std::string& name) noexcept
    : m_type(type)
    , m_size(size)
    , m_data(nullptr)

    , m_callback(callback)
    , m_name(name)
{
    switch(m_type)
    {
    case QueryType::Occlusion:
    case QueryType::TimeStamp:
        m_data = malloc(m_size * sizeof(u32)); //TODO pool ?
        break;

    default:
        ASSERT(false, "not impl");
    }
}

Query::~Query()
{
    if (m_data)
    {
        delete m_data; //TODO pool ?
    }
}

void Query::dispatch(QueryResult result) const
{
    if (m_callback)
    {
        std::invoke(m_callback, result, static_cast<u32>(m_size * sizeof(u32)), m_data);
    }
}

} //namespace renderer
} //namespace v3d
