#include "Resource.h"
#include "Stream/Stream.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace resource
{

const u16 g_resourceHeadMagicNumber = 0xF0F0;
const u16 g_resourceHeadCurrentVersion = 0x01;

bool ResourceHeader::validate(const ResourceHeader* header)
{
    ASSERT(header, "nullptr");
    return header->_head == g_resourceHeadMagicNumber;
}

void ResourceHeader::fill(ResourceHeader* header, const std::string& name, u32 size, u32 offset, u32 flags)
{
    static std::atomic<u64> IDGenerator = 1;
    u64 id = IDGenerator.fetch_add(1, std::memory_order_relaxed);
    ASSERT(id != std::numeric_limits<u64>::max(), "range out");

    ASSERT(header, "nullptr");
    header->setName(name);
    header->_size = size;
    header->_offset = offset;
    header->_flags = flags;
    header->_uID = id;
    header->_timestamp = utils::Timer::getCurrentTime();
}

ResourceHeader::ResourceHeader(ResourceType type) noexcept
    : _head(g_resourceHeadMagicNumber)
    , _type(type)
    , _version(g_resourceHeadCurrentVersion)
    , _flags(0x0)

    , _size(0)
    , _offset(0)
    , _timestamp(0)
    , _uID(0)
    , _name("")
{
    static_assert(sizeof(ResourceHeader) == 32 + k_nameSize, "wrong size");
}

ResourceHeader::ResourceHeader(const ResourceHeader& other) noexcept
    : _head(other._head)
    , _type(other._type)
    , _version(other._version)
    , _flags(other._flags)
    , _size(other._size)
    , _offset(other._offset)
    , _timestamp(other._timestamp)
    , _uID(other._uID)
{
    ASSERT(ResourceHeader::validate(this), "wrong header");
    memcpy(_name, other._name, k_nameSize);
}

void ResourceHeader::setName(const std::string& name)
{
    ASSERT(name.size() < k_nameSize, "max size is limited");
    memcpy(_name, name.c_str(), std::min<u64>(name.size(), k_nameSize));
}

u32 ResourceHeader::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();
    ASSERT(ResourceHeader::validate(this), "wrong header");
    stream->write<u16>(_head);

    stream->write<ResourceType>(_type);
    stream->write<u16>(_version);
    stream->write<u16>(_flags);

    stream->write<u32>(_offset);
    stream->write<u32>(_size);

    stream->write<u64>(_timestamp);
    stream->write<u64>(_uID);
    stream->write(_name, sizeof(u8), k_nameSize);

    u32 writeSize = stream->tell() - writePos;
    ASSERT(sizeof(ResourceHeader) == writeSize, "wrong size");
    return writeSize;
}

u32 ResourceHeader::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();
    stream->read<u16>(_head);
    ASSERT(ResourceHeader::validate(this), "wrong header");

    stream->read<ResourceType>(_type);
    stream->read<u16>(_version);
    stream->read<u16>(_flags);

    stream->read<u32>(_offset);
    stream->read<u32>(_size);

    stream->read<u64>(_timestamp);
    stream->read<u64>(_uID);
    stream->read(_name, sizeof(u8), k_nameSize);

    u32 readSize = stream->tell() - readPos;
    ASSERT(sizeof(ResourceHeader) == readSize, "wrong size");
    return readSize;
}

} //manespace resource
} //namespace v3d