#pragma once

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief QueryType enum
    */
    enum class QueryType : u32
    {
        Occlusion = 0,
        BinaryOcclusion,
        PipelineStaticstic,
        TimeStamp,

        First = Occlusion,
        Last = TimeStamp,
        Count
    };

    /**
    * @brief QueryStatus enum
    */
    enum class QueryStatus
    {
        Ready,
        Started,
        Ended,
        Submited
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