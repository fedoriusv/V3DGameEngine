#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace stream
{
    class Stream;
} //namespace stream

namespace renderer
{
    class Buffer;
    class VertexStreamBuffer;
    class IndexStreamBuffer;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxVertexInputAttributes = 16;
    constexpr u32 k_maxVertexInputBindings = 4;

    /**
    * StreamBufferUsage enum. usageFlag inside StreamBuffer
    */
    enum StreamBufferUsage
    {
        StreamBuffer_Write = 0x01,
        StreamBuffer_Read = 0x02,

        StreamBuffer_Shared = 0x04,
        StreamBuffer_Dynamic = 0x08,
    };

    typedef u16 StreamBufferUsageFlags;

    enum StreamIndexBufferType : u32
    {
        IndexType_16,
        IndexType_32
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VertexInputAttributeDesc struct
    */
    struct VertexInputAttribDescription
    {
        enum InputRate : u32
        {
            InputRate_Vertex = 0,
            InputRate_Instance = 1
        };

        struct InputBinding
        {
            InputBinding() noexcept;
            InputBinding(u32 index, InputRate rate, u32 stride) noexcept;

            void operator >> (stream::Stream* stream);
            void operator << (const stream::Stream* stream);

            u32       _index;
            InputRate _rate;
            u32       _stride;
        };

        struct InputAttribute
        {
            InputAttribute() noexcept;
            InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept;

            void operator >> (stream::Stream* stream);
            void operator << (const stream::Stream* stream);

            u32          _bindingId;
            u32          _streamId;
            Format       _format;
            u32          _offest;
        };

        VertexInputAttribDescription() noexcept;
        VertexInputAttribDescription(const VertexInputAttribDescription& desc) noexcept;
        VertexInputAttribDescription(std::vector<InputBinding> inputBindings, std::vector<VertexInputAttribDescription::InputAttribute> inputAttributes) noexcept;
        VertexInputAttribDescription& operator=(const VertexInputAttribDescription& desc);

        void operator >> (stream::Stream* stream);
        void operator << (const stream::Stream* stream);

        u32 _countInputBindings;
        std::array<InputBinding, k_maxVertexInputBindings> _inputBindings;

        u32 _countInputAttributes;
        std::array<InputAttribute, k_maxVertexInputAttributes> _inputAttribute;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferDescription struct
    */
    struct StreamBufferDescription
    {
        StreamBufferDescription() noexcept = default;
        explicit StreamBufferDescription(const StreamBufferDescription& desc) noexcept;
        explicit StreamBufferDescription(StreamBufferDescription&& desc) noexcept;
        StreamBufferDescription& operator=(StreamBufferDescription&& desc);

        explicit StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream) noexcept;
        explicit StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream, u32 offset) noexcept;

        explicit StreamBufferDescription(IndexStreamBuffer* index, u32 indexOffset, VertexStreamBuffer* vertex, u32 vertexOffset, u32 stream) noexcept;

        ~StreamBufferDescription();

        bool operator==(const StreamBufferDescription& desc);
        bool operator!=(const StreamBufferDescription& desc);

        std::vector<Buffer*> _vertices;
        std::vector<u32>     _streamsID;
        std::vector<u64>     _offsets;

        Buffer*               _indices;
        u32                   _indicesOffet;
        StreamIndexBufferType _indexType;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
