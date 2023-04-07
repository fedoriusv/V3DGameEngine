#include "Resource.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace resource
{

ResourceHeader::ResourceHeader() noexcept
    : _size(0)
    , _offset(0)
    , _version(0)
    , _extraFlags(0)
#if DEBUG
    , _name("")
#endif
{
}

u32 ResourceHeader::operator>>(stream::Stream* stream)
{
    u32 write = 0;
    write += stream->write<u32>(_size);
    write += stream->write<u32>(_offset);
    write += stream->write<u32>(_version);
    write += stream->write<u32>(_extraFlags);
#if DEBUG
    write += stream->write(_name);
#endif //DEBUG

    return write;
}

u32 ResourceHeader::operator<<(const stream::Stream* stream)
{
    u32 read = 0;
    read += stream->read<u32>(_size);
    read += stream->read<u32>(_offset);
    read += stream->read<u32>(_version);
    read += stream->read<u32>(_extraFlags);
#if DEBUG
    read += stream->read(_name);
#endif //DEBUG

    return read;
}

Resource::Resource() noexcept
    : m_header(nullptr)
    , m_stream(nullptr)
    , m_loaded(false)
{
}

Resource::Resource(const ResourceHeader* header) noexcept
    : m_header(header)
    , m_stream(nullptr)
    , m_loaded(false)
{
}

Resource::~Resource()
{
    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryDefault);
    }

    if (m_stream)
    {
        ASSERT(!m_stream->isMapped(), "mapped");
        V3D_DELETE(m_stream, memory::MemoryLabel::MemoryDefault);
    }
}

} //manespace resource
} //namespace v3d