#include "StreamBuffer.h"

namespace v3d
{
namespace renderer
{

//VertexStreamBuffer::VertexStreamBuffer(CommandList& cmdList, StreamType type, const std::vector<Stream>& streams) noexcept
//    : m_cmdList(cmdList)
//    , m_type(type)
//    , m_streams(streams)
//{
//}

VertexStreamBuffer::VertexStreamBuffer(renderer::CommandList& cmdList, VertexStreamBuffer::StreamType type, u32 size, void* data) noexcept
    : m_cmdList(cmdList)
    , m_type(type)
{
    m_streams.push_back({ size, data });
}

VertexStreamBuffer::~VertexStreamBuffer()
{
}

} //namespace renderer
} //namespace v3d
