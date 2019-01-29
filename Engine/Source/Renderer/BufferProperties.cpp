#include "BufferProperties.h"
#include "Object/StreamBuffer.h"

namespace v3d
{
namespace renderer
{

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream) noexcept
    : _vertices({ vertex->m_buffer })
    , _streamsID({ stream })
{
    _offsets.resize(_vertices.size(), 0 );
}

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream, u32 offset) noexcept
    : _vertices({ vertex->m_buffer })
    , _streamsID({ stream })
    , _offsets({ offset })
{
}

StreamBufferDescription::~StreamBufferDescription()
{
}

StreamBufferDescription::StreamBufferDescription(const StreamBufferDescription & desc) noexcept
    : _vertices(desc._vertices)
    , _streamsID(desc._streamsID)
    , _offsets(desc._offsets)
{
}

StreamBufferDescription::StreamBufferDescription(StreamBufferDescription && desc) noexcept
{
    _vertices = std::move(desc._vertices);
    _streamsID = std::move(desc._streamsID);
    _offsets = std::move(desc._offsets);
}

StreamBufferDescription & StreamBufferDescription::operator=(StreamBufferDescription && desc)
{
    if (this == &desc)
    {
        return *this;
    }

    _vertices = std::move(desc._vertices);
    _streamsID = std::move(desc._streamsID);
    _offsets = std::move(desc._offsets);

    return *this;
}

bool StreamBufferDescription::operator==(const StreamBufferDescription & desc)
{
    if (this == &desc)
    {
        return true;
    }

    if (_vertices == desc._vertices &&
        _streamsID == desc._streamsID &&
        _offsets == desc._offsets)
    {
        return true;
    }

    return false;
}

bool StreamBufferDescription::operator!=(const StreamBufferDescription & desc)
{
    return !StreamBufferDescription::operator==(desc);
}

} //namespace renderer
} //namespace v3d
