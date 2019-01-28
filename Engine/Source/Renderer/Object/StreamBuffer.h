#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/CommandList.h"
#include "Renderer/Formats.h"
#include "Renderer/BufferProperties.h"

namespace v3d
{
namespace renderer
{
    class Buffer;
    struct StreamBufferDescription;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
    */
    class VertexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;
        ~VertexStreamBuffer();

        bool update(u32 offset, u64 size, void* data);

        //map
        //unmap

    private:

        void handleNotify(utils::Observable* ob) override;

        explicit VertexStreamBuffer(CommandList& cmdList, u16 usageFlag, u64 size, const void* data) noexcept;

        friend CommandList;
        CommandList& m_cmdList;


        u64                 m_size;
        void*               m_data;

        u16                 m_usageFlag;
        Buffer*             m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
