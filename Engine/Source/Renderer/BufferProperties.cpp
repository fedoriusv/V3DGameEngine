#include "BufferProperties.h"
#include "Object/StreamBuffer.h"

namespace v3d
{
namespace renderer
{

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 firstVertex, u32 countVertex) noexcept
    : _vertices({ vertex->m_buffer })
    , _firstVertex(firstVertex)
    , _countVertex(countVertex)
{
}

StreamBufferDescription::~StreamBufferDescription()
{
}

} //namespace renderer
} //namespace v3d
