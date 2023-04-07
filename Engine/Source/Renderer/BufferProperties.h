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

    constexpr u32 k_maxVertexInputAttributes = 8;
    constexpr u32 k_maxVertexInputBindings = 4;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StreamBufferUsage enum. usageFlag inside StreamBuffer
    */
    enum StreamBufferUsage
    {
        StreamBuffer_Write = 0x01,  //Write to buffer
        StreamBuffer_Read = 0x02,   //Read from buffer

        StreamBuffer_Shared = 0x04, //Share data between Game / Render side
        StreamBuffer_Dynamic = 0x08,//Stream read/write buffer
    };

    typedef u16 StreamBufferUsageFlags;

    enum StreamIndexBufferType : u32
    {
        IndexType_16,
        IndexType_32
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexInputAttributeDescription struct
    */
    struct VertexInputAttributeDescription
    {
        enum InputRate : u32
        {
            InputRate_Vertex = 0,
            InputRate_Instance = 1
        };

        /**
        * @brief InputBinding struct
        */
        struct InputBinding
        {
            InputBinding() noexcept;
            InputBinding(u32 index, InputRate rate, u32 stride) noexcept;

            u32 operator>>(stream::Stream* stream);
            u32 operator<<(const stream::Stream* stream);

            u32       _index;
            InputRate _rate;
            u32       _stride;
        };

        /**
        * @brief InputAttribute struct
        */
        struct InputAttribute
        {
            InputAttribute() noexcept;
            InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept;

            u32 operator>>(stream::Stream* stream);
            u32 operator<<(const stream::Stream* stream);

            u32          _bindingId;
            u32          _streamId;
            Format       _format;
            u32          _offest;
        };

        VertexInputAttributeDescription() noexcept;
        VertexInputAttributeDescription(const VertexInputAttributeDescription& desc) noexcept;
        VertexInputAttributeDescription(std::vector<InputBinding> inputBindings, std::vector<VertexInputAttributeDescription::InputAttribute> inputAttributes) noexcept;
        VertexInputAttributeDescription& operator=(const VertexInputAttributeDescription& desc) noexcept;

        bool operator==(const VertexInputAttributeDescription& desc);

        u32 operator>>(stream::Stream* stream);
        u32 operator<<(const stream::Stream* stream);


        u32 _countInputBindings;
        std::array<InputBinding, k_maxVertexInputBindings> _inputBindings;

        u32 _countInputAttributes;
        std::array<InputAttribute, k_maxVertexInputAttributes> _inputAttributes;

    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StreamBufferDescription struct
    */
    struct StreamBufferDescription
    {
        StreamBufferDescription() = default;
        ~StreamBufferDescription() = default;

        explicit StreamBufferDescription(const StreamBufferDescription& desc) noexcept;
        explicit StreamBufferDescription(StreamBufferDescription&& desc) noexcept;
        StreamBufferDescription& operator=(const StreamBufferDescription& desc) noexcept;
        StreamBufferDescription& operator=(StreamBufferDescription&& desc) noexcept;

        explicit StreamBufferDescription(const VertexStreamBuffer* vertex, u32 stream) noexcept;
        explicit StreamBufferDescription(const VertexStreamBuffer* vertex, u32 stream, u32 offset) noexcept;

        explicit StreamBufferDescription(const IndexStreamBuffer* index, u32 indexOffset, const VertexStreamBuffer* vertex, u32 vertexOffset, u32 stream) noexcept;

        void clear();

        bool operator==(const StreamBufferDescription& desc);
        bool operator!=(const StreamBufferDescription& desc);

        std::vector<Buffer*> _vertices;
        std::vector<u32>     _streamsID;
        std::vector<u64>     _offsets;

        Buffer*               _indices = nullptr;
        u32                   _indicesOffet = 0;
        StreamIndexBufferType _indexType = IndexType_16;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief DrawProperties struct
    */
    struct DrawProperties
    {
        u32 _start;
        u32 _count;
        u32 _startInstance;
        u32 _countInstance;

        bool _indexDraws;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
