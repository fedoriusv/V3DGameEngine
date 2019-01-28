#include "BufferProperties.h"
#include "Object/StreamBuffer.h"

namespace v3d
{
namespace renderer
{

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream) noexcept
    : _vertices({ {vertex->m_buffer, stream} })
{
    _offsets.resize(_vertices.size(), { 0, 0 });
}

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream, u32 offset, u32 size) noexcept
    : _vertices({ {vertex->m_buffer, stream} })
    , _offsets({ { offset, size } })
{
}

StreamBufferDescription::~StreamBufferDescription()
{
}

} //namespace renderer
} //namespace v3d
