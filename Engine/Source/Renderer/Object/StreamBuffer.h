#pragma once

#include "Common.h"
#include "Object.h"
#include "Renderer/CommandList.h"
#include "Renderer/Formats.h"

namespace v3d
{
namespace renderer
{
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
    class VertexStreamBuffer : public StreamBuffer
    {
    public:

        struct Stream
        {
            Stream()
                : _size(0)
                , _data(0)
             //   , _lock(false)
            {
            }

            Stream(u32 size, void* data)
                : _size(size)
                , _data(data)
              //  , _lock(false)
            {
            }

            u32                 _size;
            void*               _data;
            //std::atomic<bool>   _lock;
        };

        enum StreamType
        {
            StreamType_Static,
            StreamType_Dynamic,
        };

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;
        ~VertexStreamBuffer();

    private:

        //explicit VertexStreamBuffer(CommandList& cmdList, StreamType type, const std::vector<Stream>& streams) noexcept;
        explicit VertexStreamBuffer(CommandList& cmdList, VertexStreamBuffer::StreamType type, u32 size, void* data) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        StreamType m_type;
        std::vector<Stream> m_streams;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
