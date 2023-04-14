#include "BufferProperties.h"
#include "StreamBuffer.h"
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

StreamBufferDescription::StreamBufferDescription(const StreamBufferDescription & desc) noexcept
    : _vertices(desc._vertices)
    , _streamsID(desc._streamsID)
    , _offsets(desc._offsets)

    , _indices(desc._indices)
    , _indicesOffet(desc._indicesOffet)
    , _indexType(desc._indexType)
{
}

StreamBufferDescription::StreamBufferDescription(StreamBufferDescription&& desc) noexcept
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

StreamBufferDescription& StreamBufferDescription::operator=(StreamBufferDescription&& desc) noexcept
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

bool StreamBufferDescription::operator==(const StreamBufferDescription& desc) const
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

bool StreamBufferDescription::operator!=(const StreamBufferDescription& desc) const
{
    return !StreamBufferDescription::operator==(desc);
}

VertexInputAttributeDescription::VertexInputAttributeDescription() noexcept
    : _countInputBindings(0)
    , _countInputAttributes(0)
{
    static_assert(sizeof(VertexInputAttributeDescription) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 8, "wrong size");
}

VertexInputAttributeDescription::VertexInputAttributeDescription(const VertexInputAttributeDescription& desc) noexcept
    : _countInputBindings(desc._countInputBindings)
    , _inputBindings(desc._inputBindings)
    , _countInputAttributes(desc._countInputAttributes)
    , _inputAttributes(desc._inputAttributes)
{
    static_assert(sizeof(VertexInputAttributeDescription) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 8, "wrong size");
}

VertexInputAttributeDescription::VertexInputAttributeDescription(std::vector<InputBinding> inputBindings, std::vector<VertexInputAttributeDescription::InputAttribute> inputAttributes) noexcept
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

VertexInputAttributeDescription& VertexInputAttributeDescription::operator=(const VertexInputAttributeDescription& desc) noexcept
{
    _countInputBindings = desc._countInputBindings;
    _inputBindings = desc._inputBindings;

    _countInputAttributes = desc._countInputAttributes;
    _inputAttributes = desc._inputAttributes;

    return *this;
}

bool VertexInputAttributeDescription::operator==(const VertexInputAttributeDescription& desc) const
{
    if (_countInputBindings != desc._countInputBindings || _countInputAttributes != desc._countInputAttributes)
    {
        return false;
    }

    return memcmp(_inputBindings.data(), desc._inputBindings.data(), sizeof(_inputBindings)) == 0 || 
        memcmp(_inputAttributes.data(), desc._inputAttributes.data(), sizeof(_inputAttributes)) == 0;
}

u32 VertexInputAttributeDescription::operator>>(stream::Stream* stream) const
{
    u32 writeSize = 0;
    u32 writePos = stream->tell();

    stream->write<u32>(_countInputBindings);
    writeSize += sizeof(u32);
    for (u32 i = 0; i < _countInputBindings; ++i)
    {
        writeSize += _inputBindings[i] >> stream;
    }

    stream->write<u32>(_countInputAttributes);
    writeSize += sizeof(u32);
    for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        writeSize += _inputAttributes[i] >> stream;
    }

    writePos = stream->tell() - writePos;
    ASSERT(writePos == writeSize, "wrong size");
    return writeSize;
}

u32 VertexInputAttributeDescription::operator<<(const stream::Stream* stream)
{
    u32 readSize = 0;
    u32 readPos = stream->tell();

    stream->read<u32>(_countInputBindings);
    readSize += sizeof(u32);
    for (u32 i = 0; i < _countInputBindings; ++i)
    {
        readSize += _inputBindings[i] << stream;
    }

    stream->read<u32>(_countInputAttributes);
    readSize += sizeof(u32);
    for (u32 i = 0; i < _countInputAttributes; ++i)
    {
        readSize += _inputAttributes[i] << stream;
    }

    readPos = stream->tell() - readPos;
    ASSERT(readPos == readSize, "wrong size");
    return readSize;
}

VertexInputAttributeDescription::InputAttribute::InputAttribute() noexcept
{
    static_assert(sizeof(InputAttribute) == 16, "wrong size");
    memset(this, 0, sizeof(InputAttribute));
}

VertexInputAttributeDescription::InputAttribute::InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept
    : _bindingID(binding)
    , _streamId(stream)
    , _format(format)
    , _offest(offset)
{
    static_assert(sizeof(InputAttribute) == 16, "wrong size");
}

u32 VertexInputAttributeDescription::InputAttribute::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_bindingID);
    stream->write<u32>(_streamId);
    stream->write<Format>(_format);
    stream->write<u32>(_offest);

    u32 writeSize = stream->tell() - writePos;
    ASSERT(sizeof(InputAttribute) == writeSize, "wrong size");
    return writeSize;
}

u32 VertexInputAttributeDescription::InputAttribute::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_bindingID);
    stream->read<u32>(_streamId);
    stream->read<Format>(_format);
    stream->read<u32>(_offest);

    u32 readSize = stream->tell() - readPos;
    ASSERT(sizeof(InputAttribute) == readSize, "wrong size");
    return readSize;
}

VertexInputAttributeDescription::InputBinding::InputBinding() noexcept
{
    static_assert(sizeof(InputBinding) == 8, "wrong size");
    memset(this, 0, sizeof(InputBinding));
}

VertexInputAttributeDescription::InputBinding::InputBinding(u32 index, InputRate rate, u16 stride) noexcept
    : _index(index)
    , _rate(rate)
    , _stride(stride)
{
    static_assert(sizeof(InputBinding) == 8, "wrong size");
}

u32 VertexInputAttributeDescription::InputBinding::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_index);
    stream->write<InputRate>(_rate);
    stream->write<u16>(_stride);

    u32 writeSize = stream->tell() - writePos;
    ASSERT(sizeof(InputBinding) == writeSize, "wrong size");
    return writeSize;
}

u32 VertexInputAttributeDescription::InputBinding::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_index);
    stream->read<InputRate>(_rate);
    stream->read<u16>(_stride);

    u32 readSize = stream->tell() - readPos;
    ASSERT(sizeof(InputBinding) == readSize, "wrong size");
    return readSize;
}

} //namespace renderer
} //namespace v3d
