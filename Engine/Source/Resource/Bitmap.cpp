#include "Bitmap.h"
#include "Stream/Stream.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{

BitmapHeader::BitmapHeader() noexcept
    : ResourceHeader(resource::ResourceType::BitmapResource)
    , _dimension(math::Dimension3D())
    , _format(renderer::Format::Format_Undefined)
    , _layers(1)
    , _mips(1)
    , _bitmapFlags(0x0)
{
    static_assert(sizeof(BitmapHeader) == sizeof(ResourceHeader) + 24, "range out");
}

BitmapHeader::BitmapHeader(const BitmapHeader& other) noexcept
    : ResourceHeader(other)
    , _dimension(other._dimension)
    , _format(other._format)
    , _layers(other._layers)
    , _mips(other._mips)
    , _bitmapFlags(other._bitmapFlags)
{
}

u32 BitmapHeader::operator>>(stream::Stream* stream)
{
    u32 parent = ResourceHeader::operator>>(stream);
    u32 write = stream->tell();

    stream->write<math::Dimension3D>(_dimension);
    stream->write<renderer::Format>(_format);
    stream->write<u16>(_layers);
    stream->write<u16>(_mips);
    stream->write<BitmapHeaderFlags>(_bitmapFlags);

    write = stream->tell() - write + parent;
    ASSERT(sizeof(BitmapHeader) == write, "wrong size");
    return write;
}

u32 BitmapHeader::operator<<(const stream::Stream* stream)
{
    u32 parent = ResourceHeader::operator<<(stream);
    u32 read = stream->tell();

    stream->read<math::Dimension3D>(_dimension);
    stream->read<renderer::Format>(_format);
    stream->read<u16>(_layers);
    stream->read<u16>(_mips);
    stream->read<BitmapHeaderFlags>(_bitmapFlags);

    read = stream->tell() - read + parent;
    ASSERT(sizeof(BitmapHeader) == read, "wrong size");
    return read;
}

Bitmap::Bitmap() noexcept
    : m_header(nullptr)
    , m_bitmap(nullptr)
    , m_size(0)
{
    LOG_DEBUG("Bitmap constructor %llx", this);
}

Bitmap::Bitmap(BitmapHeader* header) noexcept
    : m_header(header)
    , m_bitmap(nullptr)
    , m_size(0)
{
    LOG_DEBUG("Bitmap constructor %llx", this);
}

Bitmap::~Bitmap()
{
    LOG_DEBUG("Bitmap destructor %llx", this);

    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryResource);
        m_header = nullptr;
    }

    if (m_bitmap)
    {
        V3D_DELETE(m_bitmap, memory::MemoryLabel::MemoryResource);
        m_bitmap = nullptr;
    }
}

bool Bitmap::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Bitmap::load: the bitmap %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "stream is nullptr");
    stream->seekBeg(offset);

    if (!m_header)
    {
        m_header = V3D_NEW(BitmapHeader, memory::MemoryLabel::MemoryResource);
        ASSERT(m_header, "nullptr");
        m_header->operator<<(stream);
    }
    stream->seekBeg(offset + m_header->_offset);

    stream->read<u32>(m_size);
    if (m_size > 0)
    {
        m_bitmap = reinterpret_cast<u8*>(V3D_MALLOC(m_size, memory::MemoryLabel::MemoryResource));
        stream->read(m_bitmap, m_size, sizeof(u8));
    }
    LOG_DEBUG("Bitmap::load: The stream has been read %d from %d bytes", stream->tell() - m_header->_offset, m_header->_size);

    m_loaded = true;
    return true;
}

bool Bitmap::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

} //namespace resource
} //namespace v3d
