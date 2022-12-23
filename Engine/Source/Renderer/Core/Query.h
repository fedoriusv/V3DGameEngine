#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Renderer/QueryRequestProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Query base class. Render side
    */
    class Query: public utils::Observable
    {
    public:

        using QueryRespose = std::function<void(QueryResult result, const void* data)>;

        Query(Query&) = delete;
        ~Query() = default;

        Query(QueryType type, QueryRespose callback) noexcept;

        QueryType getType() const;
        QueryRespose getCallback() const;

    protected:

        QueryType m_type;
        QueryRespose m_callback;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
