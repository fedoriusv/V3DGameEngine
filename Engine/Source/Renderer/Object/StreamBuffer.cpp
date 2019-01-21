#include "..\BufferProperties.h"
#include "StreamBuffer.h"

namespace v3d
{
namespace renderer
{

StreamBufferData::StreamBufferData()
    : _size(0)
    , _data(0)
    , _lock(false)
{
}

StreamBufferData::StreamBufferData(u32 size, void * data)
    : _size(size)
    , _data(data)
    , _lock(false)
{
}

void StreamBufferData::lock()
{
    _lock = true;
}

void StreamBufferData::unlock()
{
    _lock = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 streamId)
{
    _stream = &vertex->getStreamBufferData(streamId);
    _stream->lock();

    _usageFlag = vertex->m_usageFlag;
}

StreamBufferDescription::~StreamBufferDescription()
{
    if (_usageFlag & ~StreamBufferUsage::StreamBuffer_Direct)
    {
        _stream->unlock();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VertexStreamBuffer::VertexStreamBuffer(CommandList& cmdList, u16 usageFlag, const std::vector<StreamBufferData>& streams) noexcept
    : m_cmdList(cmdList)
    , m_usageFlag(usageFlag)
    , m_streams(streams)
{

}

VertexStreamBuffer::VertexStreamBuffer(renderer::CommandList& cmdList, u16 usageFlag, u32 size, void* data) noexcept
    : m_cmdList(cmdList)
    , m_usageFlag(usageFlag)

    , m_buffer(nullptr)
{
    m_streams.push_back({ size, data });

   /* m_buffer = m_cmdList.getContext()->createBuffer();
    ASSERT(m_buffer, "m_buffer is nullptr");
    m_buffer->registerNotify(this);*/
}

VertexStreamBuffer::~VertexStreamBuffer()
{
    //TODO
}

StreamBufferData& VertexStreamBuffer::getStreamBufferData(u32 stream) const
{
    ASSERT(stream < m_streams.size(), "range out");
    return m_streams[stream];
}

void VertexStreamBuffer::update(u32 stream, u32 size, void * data)
{
    //TODO:
}


} //namespace renderer
} //namespace v3d
