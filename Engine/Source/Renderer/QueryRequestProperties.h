#pragma once

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryType enum
    */
    enum class QueryType
    {
        First = 0,

        Occlusion = First,
        BinaryOcclusion,
        PipelineStaticstic,
        TimeStamp,

        Count
    };

    /**
    * @brief QueryResult enum
    */
    enum class QueryResult
    {
        Success
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d