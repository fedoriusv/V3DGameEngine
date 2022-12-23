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
    * @brief RenderQuery struct
    */
    struct RenderQuery
    {
        RenderQueryPool* _pool;
        u32 _index;

        RenderQuery() noexcept
            : _pool(nullptr)
            , _index(-1)
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

        RenderQueryPool(QueryType type, u32 size) noexcept;

        RenderQueryPool() = delete;
        RenderQueryPool(RenderQueryPool&) = delete;
        virtual ~RenderQueryPool();

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void reset() = 0;

        QueryType getQueryType() const;
        u32 getSize() const;

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