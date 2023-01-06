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

        Query() = delete;
        Query(Query&) = delete;

        Query(QueryType type, const QueryRespose& callback, [[maybe_unused]] const std::string& name = "") noexcept;
        virtual ~Query();

        QueryType getType() const;
        const std::string& getName() const;

        QueryRespose callback() const;

    protected:

        QueryType m_type;
        QueryRespose m_callback;
        std::string m_name;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
