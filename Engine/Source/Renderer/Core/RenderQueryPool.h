#pragma once

#include "Common.h"
#include "Renderer/QueryRequestProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderQueryPool;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderQuery struct. Render side only
    */
    struct RenderQuery
    {
        RenderQueryPool* _pool;
        u32 _index;
        u32 _used;

        RenderQuery() noexcept
            : _pool(nullptr)
            , _index(-1)
            , _used(false)
        {
        }

        RenderQuery(RenderQueryPool* pool, u32 index) noexcept
            : _pool(pool)
            , _index(index)
            , _used(false)
        {
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderQueryPool base class. Render side
    */
    class RenderQueryPool
    {
    public:

        RenderQueryPool(QueryType type, u32 poolSize) noexcept;

        RenderQueryPool() = delete;
        RenderQueryPool(RenderQueryPool&) = delete;
        virtual ~RenderQueryPool();

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void reset();

        QueryType getQueryType() const;
        u32 getPoolSize() const;
        u32 getSize() const;
        bool isFilled() const;

        RenderQuery* getRenderQuery(u32 index) const;
        std::pair<RenderQuery*, u32> takeFreeRenderQuery();

    protected:

        QueryType m_type;
        u32 m_size;

        std::vector<RenderQuery*> m_queries;
        u32 m_freeIndex;
};

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d