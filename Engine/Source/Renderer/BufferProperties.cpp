#include "BufferProperties.h"
#include "Object/StreamBuffer.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace renderer
{

StreamBufferDescription::StreamBufferDescription(const VertexStreamBuffer* vertex, u32 stream) noexcept
    : _vertices(vertex ? std::vector<Buffer*>({ vertex->m_buffer }) : std::vector<Buffer*>())
    , _streamsID({ stream })
    
    , _indices(nullptr)
    , _indicesOffet(0)
    , _indexType(StreamIndexBufferType::IndexType_16)
{
    _offsets.resize(_vertices.size(), 0 );
}

StreamBufferDescription::StreamBufferDescription(const VertexStreamBuffer* vertex, u32 stream, u32 offset) noexcept
    : _vertices(vertex ? std::vector<Buffer*>({ vertex->m_buffer }) : std::vector<Buffer*>())
    , _streamsID({ stream })
    , _offsets({ offset })

    , _indices(nullptr)
    , _indicesOffet(0)
    , _indexType(StreamIndexBufferType::IndexType_16)
{
}

StreamBufferDescription::StreamBufferDescription(const IndexStreamBuffer* index, u32 indexOffset, const VertexStreamBuffer* vertex, u32 vertexOffset, u32 stream) noexcept
    : _vertices(vertex ? std::vector<Buffer*>({ vertex->m_buffer }) : std::vector<Buffer*>())
    , _streamsID({ stream })
    , _offsets({ vertexOffset })

    , _indices(index->m_buffer)
    , _indicesOffet(indexOffset)
    , _indexType(index->m_type)
{
}

void StreamBufferDescription::clear()
{
    _vertices.clear();
    _streamsID.clear();
    _offsets.clear();

    _indices = nullptr;
    _indicesOffet = 0U;
    _indexType = StreamIndexBufferType::IndexType_16;
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

StreamBufferDescription& StreamBufferDescription::operator=(const StreamBufferDescription& desc) noexcept
{
    if (this == &desc)
    {
        return *this;
    }

    _vertices = desc._vertices;
    _streamsID = desc._streamsID;
    _offsets = desc._offsets;
    _indices = desc._indices;

    _indicesOffet = desc._indicesOffet;
    _indexType = desc._indexType;

    return *this;
}

StreamBufferDescription& StreamBufferDescription::operator=(StreamBufferDescription && desc) noexcept
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
    , _inputAttributes(desc._inputAttributes)
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
        _inputAttributes[index] = attribute;
        index++;
    }
    _countInputAttributes = index;
}

VertexInputAttribDescription & VertexInputAttribDescription::operator=(const VertexInputAttribDescription & desc) noexcept
{
    _countInputBindings = desc._countInputBindings;
    _inputBindings = desc._inputBindings;

    _countInputAttributes = desc._countInputAttributes;
    _inputAttributes = desc._inputAttributes;

    return *this;
}

bool VertexInputAttribDescription::operator==(const VertexInputAttribDescription& desc)
{
    if (_countInputBindings != desc._countInputBindings || _countInputAttributes != desc._countInputAttributes)
    {
        return false;
    }

    return memcpy(_inputBindings.data(), desc._inputBindings.data(), sizeof(_inputBindings)) || 
        memcpy(_inputAttributes.data(), desc._inputAttributes.data(), sizeof(_inputAttributes));
}

void VertexInputAttribDescription::operator>>(stream::Stream * stream)
{
    stream->write<u32>(_countInputBindings);
    stream->write(_inputBindings.data(), k_maxVertexInputBindings * sizeof(InputBinding));
    /*for (u32 i = 0; i < _countInputBindings; ++i)
    {
        _inputBindings[i] >> stream;
    }*/

    stream->write<u32>(_countInputAttributes);
    stream->write(_inputAttributes.data(), k_maxVertexInputAttributes * sizeof(InputAttribute));
    /*for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        _inputAttribute[i] >> stream;
    }*/
}

void VertexInputAttribDescription::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_countInputBindings);
    stream->read(_inputBindings.data(), k_maxVertexInputBindings *  sizeof(InputBinding));
    /*_inputBindings.fill(InputBinding());
    for (u32 i = 0; i < _countInputBindings; ++i)
    {
        _inputBindings[i] << stream;
    }*/

    stream->read<u32>(_countInputAttributes);
    stream->read(_inputAttributes.data(), k_maxVertexInputAttributes * sizeof(InputAttribute));
    /*_inputAttribute.fill(InputAttribute());
    for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        _inputAttribute[i] << stream;
    }*/
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
