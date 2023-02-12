#pragma once

#include "Common.h"
#include "Object.h"
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
    class Query : public utils::Observable
    {
    public:

        /**
        * @brief QueryRespose callback
        */
        using QueryRespose = std::function<void(QueryResult result, u32 size, const void* data)>;

        Query(QueryType type, u32 size, const QueryRespose& callback, [[maybe_unused]] const std::string& name = "") noexcept;
        virtual ~Query();

        QueryType getType() const;
        u32 getSize() const;
        const std::string& getName() const;

        void notifySelf(Object* caller); //TODO move to observer?

    protected:

        Query() = delete;
        Query(Query&) = delete;

        void dispatch(QueryResult result) const;

        QueryType m_type;
        u32 m_size;
        void* m_data;

        QueryRespose m_callback;
        std::string m_name;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline QueryType Query::getType() const
    {
        return m_type;
    }

    inline u32 Query::getSize() const
    {
        return m_size;
    }

    inline const std::string& Query::getName() const
    {
        return m_name;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
