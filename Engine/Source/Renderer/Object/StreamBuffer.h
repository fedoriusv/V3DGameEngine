#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/Formats.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class Buffer;
    struct StreamBufferDescription;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBuffer base class. Game side
    */
    class StreamBuffer : public Object
    {
    public:

        StreamBuffer() = default;
        virtual ~StreamBuffer() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VertexStreamBuffer class. Game side
    *\n
    * Constructor param:
    StreamBufferUsageFlags usage
    u64 size - in bytes
    const u8* data
    */
    class VertexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;
        ~VertexStreamBuffer();

        bool update(u32 offset, u64 size, const u8* data);
        bool read(u32 offset, u64 size, u8* data); //TODO: maybe add callback for async

    private:

        void handleNotify(utils::Observable* ob) override;

        explicit VertexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, u64 size, const u8* data) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        u64     m_size;
        void*   m_data;
                
        StreamBufferUsageFlags m_usage;
        Buffer* m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * IndexStreamBuffer class. Game side
    *\n
    * Constructor param:
    StreamBufferUsageFlags usage
    StreamIndexBufferType type
    u32 count
    const u8* data
    */
    class IndexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        IndexStreamBuffer() = delete;
        IndexStreamBuffer(const IndexStreamBuffer&) = delete;
        ~IndexStreamBuffer();

        u32 getIndexCount() const;
        StreamIndexBufferType getIndexBufferType() const;

    private:

        explicit IndexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, StreamIndexBufferType type, u32 count, const u8* data) noexcept;

        void handleNotify(utils::Observable* ob) override;

        friend CommandList;
        CommandList& m_cmdList;

        StreamIndexBufferType   m_type;
        u32         m_count;
        void*       m_data;

        StreamBufferUsageFlags m_usage;
        Buffer* m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
