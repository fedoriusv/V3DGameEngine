#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class QueryType
    {
        Occlusion,
        TimeStamp,

        Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;
    class QueryPool;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderQuery class final. Render side
    */
    class Query final
    {
    public:

        Query() noexcept;
        ~Query() = default;

    protected:

        QueryPool* m_pool;
        u32 m_index;

        friend QueryPool;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderQueryPool class. Render side
    */
    class QueryPool
    {
    public:

        explicit QueryPool(QueryType type, u32 size) noexcept;
        virtual ~QueryPool();

        virtual bool create() = 0;
        virtual void destroy() = 0;

    protected:

        QueryType m_type;
        u32 m_size;
        std::vector<Query*> m_queries;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryPoolManager class. Render side
    */
    class QueryPoolManager final
    {
    public:

        QueryPoolManager() = delete;
        QueryPoolManager(const QueryPoolManager&) = delete;

        explicit QueryPoolManager(Context* context) noexcept;
        ~QueryPoolManager();

        Query* acquireQueryPool(QueryType type);
        void removeQueryPool(QueryPool* pool);

    private:

        Context* const m_context;
        std::list<QueryPool*> m_pools[toEnumType(QueryType::Count)];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
