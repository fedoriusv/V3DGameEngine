#pragma once

#include "Common.h"
#include "Object.h"
#include "Renderer/CommandList.h"
#include "Renderer/Formats.h"
#include "Renderer/BufferProperties.h"

namespace v3d
{
namespace renderer
{
    class Buffer;

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

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;
        ~VertexStreamBuffer();

        StreamBufferData& getStreamBufferData(u32 stream) const;

        void update(u32 stream, u32 size, void* data);

    private:

        explicit VertexStreamBuffer(CommandList& cmdList, u16 usageFlag, const std::vector<StreamBufferData>& streams) noexcept;
        explicit VertexStreamBuffer(CommandList& cmdList, u16 usageFlag, u32 size, void* data) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        u16 m_usageFlag;
        mutable std::vector<StreamBufferData> m_streams;

        Buffer* m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
