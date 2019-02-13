#include "BufferProperties.h"
#include "Object/StreamBuffer.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace renderer
{

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream) noexcept
    : _vertices({ vertex->m_buffer })
    , _streamsID({ stream })
    
    , _indices(nullptr)
    , _indicesOffet(0)
    , _indexType(StreamIndexBufferType::IndexType_32)
{
    _offsets.resize(_vertices.size(), 0 );
}

StreamBufferDescription::StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream, u32 offset) noexcept
    : _vertices({ vertex->m_buffer })
    , _streamsID({ stream })
    , _offsets({ offset })

    , _indices(nullptr)
    , _indicesOffet(0)
    , _indexType(StreamIndexBufferType::IndexType_32)
{
}

StreamBufferDescription::StreamBufferDescription(IndexStreamBuffer* index, u32 indexOffset, VertexStreamBuffer* vertex, u32 vertexOffset, u32 stream) noexcept
    : _vertices({ vertex->m_buffer })
    , _streamsID({ stream })
    , _offsets({ vertexOffset })

    , _indices(index->m_buffer)
    , _indicesOffet(indexOffset)
    , _indexType(index->m_type)
{
}

StreamBufferDescription::~StreamBufferDescription()
{
}

StreamBufferDescription::StreamBufferDescription(const StreamBufferDescription & desc) noexcept
    : _vertices(desc._vertices)
    , _streamsID(desc._streamsID)
    , _offsets(desc._offsets)

    , _indices(desc._indices)
    , _indicesOffet(desc._indicesOffet)
    , _indexType(desc._indexType)
{
}

StreamBufferDescription::StreamBufferDescription(StreamBufferDescription && desc) noexcept
{
    _vertices = std::move(desc._vertices);
    _streamsID = std::move(desc._streamsID);
    _offsets = std::move(desc._offsets);

    _indices = desc._indices;
    desc._indices = nullptr;

    _indicesOffet = desc._indicesOffet;
    _indexType = desc._indexType;
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

    _indices = desc._indices;
    desc._indices = nullptr;

    _indicesOffet = desc._indicesOffet;
    _indexType = desc._indexType;

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
        _offsets == desc._offsets &&
        _indices == desc._indices &&
        _indicesOffet == desc._indicesOffet &&
        _indexType == desc._indexType)
    {
        return true;
    }

    return false;
}

bool StreamBufferDescription::operator!=(const StreamBufferDescription & desc)
{
    return !StreamBufferDescription::operator==(desc);
}

VertexInputAttribDescription::VertexInputAttribDescription() noexcept
    : _countInputBindings(0)
    , _countInputAttributes(0)
{
}

VertexInputAttribDescription::VertexInputAttribDescription(const VertexInputAttribDescription& desc) noexcept
    : _countInputBindings(desc._countInputBindings)
    , _inputBindings(desc._inputBindings)
    , _countInputAttributes(desc._countInputAttributes)
    , _inputAttribute(desc._inputAttribute)
{
}

VertexInputAttribDescription::VertexInputAttribDescription(std::vector<InputBinding> inputBindings, std::vector<VertexInputAttribDescription::InputAttribute> inputAttributes) noexcept
{
    u32 index = 0;
    for (auto& binding : inputBindings)
    {
        _inputBindings[index] = binding;
        index++;
    }
    _countInputBindings = index;

    index = 0;
    for (auto& attribute : inputAttributes)
    {
        _inputAttribute[index] = attribute;
        index++;
    }
    _countInputAttributes = index;
}

VertexInputAttribDescription & VertexInputAttribDescription::operator=(const VertexInputAttribDescription & desc)
{
    _countInputBindings = desc._countInputBindings;
    _inputBindings = desc._inputBindings;

    _countInputAttributes = desc._countInputAttributes;
    _inputAttribute = desc._inputAttribute;

    return *this;
}

void VertexInputAttribDescription::operator>>(stream::Stream * stream)
{
    stream->write<u32>(_countInputBindings);
    for (u32 i = 0; i < _countInputBindings; ++i)
    {
        _inputBindings[i] >> stream;
    }

    stream->write<u32>(_countInputAttributes);
    for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        _inputAttribute[i] >> stream;
    }
}

void VertexInputAttribDescription::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_countInputBindings);
    _inputBindings.fill(InputBinding());
    for (u32 i = 0; i < _countInputBindings; ++i)
    {
        _inputBindings[i] << stream;
    }

    stream->read<u32>(_countInputAttributes);
    _inputAttribute.fill(InputAttribute());
    for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        _inputAttribute[i] << stream;
    }
}

VertexInputAttribDescription::InputAttribute::InputAttribute() noexcept
{
    memset(this, 0, sizeof(InputAttribute));
}

VertexInputAttribDescription::InputAttribute::InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept
    : _bindingId(binding)
    , _streamId(stream)
    , _format(format)
    , _offest(offset)
{
}

void VertexInputAttribDescription::InputAttribute::operator>>(stream::Stream * stream)
{
    stream->write<u32>(_bindingId);
    stream->write<u32>(_streamId);
    stream->write<Format>(_format);
    stream->write<u32>(_offest);
}

void VertexInputAttribDescription::InputAttribute::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_bindingId);
    stream->read<u32>(_streamId);
    stream->read<Format>(_format);
    stream->read<u32>(_offest);
}

VertexInputAttribDescription::InputBinding::InputBinding() noexcept
{
    memset(this, 0, sizeof(InputBinding));
}

VertexInputAttribDescription::InputBinding::InputBinding(u32 index, InputRate rate, u32 stride) noexcept
    : _index(index)
    , _rate(rate)
    , _stride(stride)
{
}

void VertexInputAttribDescription::InputBinding::operator>>(stream::Stream * stream)
{
    stream->write<u32>(_index);
    stream->write<InputRate>(_rate);
    stream->write<u32>(_stride);
}

void VertexInputAttribDescription::InputBinding::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_index);
    stream->read<InputRate>(_rate);
    stream->read<u32>(_stride);
}

} //namespace renderer
} //namespace v3d
