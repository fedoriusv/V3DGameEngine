#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    class Buffer;
    class VertexStreamBuffer;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferDescription struct
    */
    struct StreamBufferDescription
    {
        StreamBufferDescription() = delete;
        explicit StreamBufferDescription(const StreamBufferDescription& desc) = default;

        StreamBufferDescription(VertexStreamBuffer* vertex, u32 firstVertex, u32 countVertex) noexcept;
        ~StreamBufferDescription();


        std::vector<Buffer*> _vertices;
        std::vector<u64>     _offsets;
        u32 _firstVertex;
        u32 _countVertex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
