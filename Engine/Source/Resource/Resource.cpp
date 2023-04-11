#include "Resource.h"
#include "Stream/Stream.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace resource
{

const u16 g_resourceHeadMagicNumber = 0xF0F0;

bool ResourceHeader::validateResourceHeader(const ResourceHeader* header)
{
    ASSERT(header, "nullptr");
    return header->_head == g_resourceHeadMagicNumber;
}

void ResourceHeader::fillResourceHeader(ResourceHeader* header, const std::string& name, u32 size, u32 offset, u32 flags)
{
    static std::atomic<u64> IDGenerator = 1;
    u64 id = IDGenerator.fetch_add(1, std::memory_order_relaxed);
    ASSERT(id != std::numeric_limits<u64>::max(), "range out");

    ASSERT(header, "nullptr");
    header->setName(name);
    header->_size = size;
    header->_offset = offset;
    header->_extraFlags = flags;
    header->_unId = id;
    header->_timestamp = utils::Timer::getCurrentTime();
}

ResourceHeader::ResourceHeader(ResourceType type) noexcept
    : _head(g_resourceHeadMagicNumber)
    , _type(ResourceType::EmptyResource)
    , _version(0x0000)
    , _extraFlags(0x0)
    , _size(0)
    , _offset(0)
    , _timestamp(0)
    , _unId(0)
    , _name("")
{
    static_assert(sizeof(ResourceHeader) == 32 + k_nameSize, "wrong size");
}

ResourceHeader::ResourceHeader(const ResourceHeader& other) noexcept
    : _head(other._head)
    , _type(other._type)
    , _version(other._version)
    , _extraFlags(other._extraFlags)
    , _size(other._size)
    , _offset(other._offset)
    , _timestamp(other._timestamp)
    , _unId(other._unId)
{
    ASSERT(ResourceHeader::validateResourceHeader(this), "wrong header");
    memcpy(_name, other._name, k_nameSize);
}

void ResourceHeader::setName(const std::string& name)
{
    ASSERT(name.size() < k_nameSize, "max size is limited");
    memcpy(_name, name.c_str(), math::min<u64>(name.size(), k_nameSize));
}

u32 ResourceHeader::operator>>(stream::Stream* stream)
{
    u32 write = 0;
    ASSERT(ResourceHeader::validateResourceHeader(this), "wrong header");
    write += stream->write<u16>(_head);

    write += stream->write<ResourceType>(_type);
    write += stream->write<u16>(_version);
    write += stream->write<u32>(_extraFlags);

    write += stream->write<u32>(_offset);
    write += stream->write<u32>(_size);

    write += stream->write<u64>(_timestamp);
    write += stream->write<u64>(_unId);
    write += stream->write(_name, sizeof(u8), k_nameSize);

    ASSERT(sizeof(ResourceHeader) == write, "wrong size");
    return write;
}

u32 ResourceHeader::operator<<(const stream::Stream* stream)
{
    u32 read = 0;
    read += stream->read<u16>(_head);
    ASSERT(ResourceHeader::validateResourceHeader(this), "wrong header");

    read += stream->read<ResourceType>(_type);
    read += stream->read<u16>(_version);
    read += stream->read<u16>(_extraFlags);

    read += stream->read<u32>(_offset);
    read += stream->read<u32>(_size);

    read += stream->read<u64>(_timestamp);
    read += stream->read<u64>(_unId);
    read += stream->read(_name, sizeof(u8), k_nameSize);

    ASSERT(sizeof(ResourceHeader) == read, "wrong size");
    return read;
}

} //manespace resource
} //namespace v3d