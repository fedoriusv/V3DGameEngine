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

        explicit Query(QueryType type, u32 count, const QueryRespose& callback) noexcept;
        virtual ~Query();

        virtual bool create() = 0;
        virtual void destroy() = 0;

        QueryType getType() const;
        u32 getCount() const;

        void notifySelf(Object* caller); //TODO move to observer?

    protected:

        Query() = delete;
        Query(Query&) = delete;

        QueryType m_type;
        u32 m_count;
        QueryRespose m_callback;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline QueryType Query::getType() const
    {
        return m_type;
    }

    inline u32 Query::getCount() const
    {
        return m_count;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
