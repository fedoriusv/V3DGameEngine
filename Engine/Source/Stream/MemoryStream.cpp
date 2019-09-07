#include "MemoryStream.h"
#include "Utils/MemoryPool.h"

namespace v3d
{
namespace stream
{

MemoryStream::MemoryStream(utils::MemoryPool* allocator) noexcept
    : m_stream(nullptr)
    , m_length(0)
    , m_allocated(0)
    , m_pos(0)
    , m_mapped(false)
    , m_allocator(allocator)
{
}

MemoryStream::MemoryStream(const MemoryStream& stream, utils::MemoryPool* allocator) noexcept
    : m_stream(nullptr)
    , m_length(stream.m_length)
    , m_allocated(stream.m_allocated)
    , m_pos(stream.m_pos)
    , m_mapped(false)
    , m_allocator(allocator)
{
    ASSERT(!m_mapped, "data is mapped");
    if (stream.m_stream && m_allocated > 0)
    {
        MemoryStream::allocate(m_allocated);
        memcpy(m_stream, stream.m_stream, m_length);
    }
}

MemoryStream::MemoryStream(MemoryStream&& stream, utils::MemoryPool* allocator) noexcept
    : m_stream(nullptr)
    , m_length(0)
    , m_allocated(0)
    , m_pos(0)
    , m_mapped(false)
    , m_allocator(allocator)
{
    std::swap(m_stream, stream.m_stream);
    std::swap(m_length, stream.m_length);
    std::swap(m_allocated, stream.m_allocated);
    std::swap(m_pos, stream.m_pos);
    ASSERT(!m_mapped, "data is mapped");
}

MemoryStream::MemoryStream(const void* data, u32 size, utils::MemoryPool* allocator) noexcept
    : m_stream(nullptr)
    , m_length(size)
    , m_allocated(size)
    , m_pos(0)
    , m_mapped(false)
    , m_allocator(allocator)
{
    if (m_length > 0)
    {
        MemoryStream::allocate(size);
        if (data)
        {
            ASSERT(m_stream, "nullptr");
            memcpy(m_stream, data, m_length);
        }
    }
}

MemoryStream::~MemoryStream()
{
    clear();
}

void MemoryStream::close()
{
}

u32 MemoryStream::read(void* buffer, u32 size, u32 count) const
{
    ASSERT(m_pos + size * count <= m_length, "Invalid memory size");

    memcpy(buffer, m_stream + m_pos, size * count);
    m_pos += size * count;

    return m_pos;
}

u32 MemoryStream::read(u8& value) const
{
    ASSERT(m_length - m_pos >= sizeof(u8), "Invalid memory size");

    value = m_stream[m_pos++];

    return m_pos;
}

u32 MemoryStream::read(s8& value) const
{
    ASSERT(m_length - m_pos >= sizeof(s8), "Invalid memory size");

    value = m_stream[m_pos++];

    return m_pos;
}

u32 MemoryStream::read(u16& value) const
{
    ASSERT(m_length - m_pos >= sizeof(u16), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFF) << 8;
    value |= m_stream[m_pos++] & 0xFF;

    return m_pos;
}

u32 MemoryStream::read(s16& value) const
{
    ASSERT(m_length - m_pos >= sizeof(s16), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFF) << 8;
    value |= m_stream[m_pos++] & 0xFF;

    return m_pos;
}

u32 MemoryStream::read(u32& value) const
{
    ASSERT(m_length - m_pos >= sizeof(u32), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFF) << 24;
    value |= (m_stream[m_pos++] & 0xFF) << 16;
    value |= (m_stream[m_pos++] & 0xFF) << 8;
    value |= (m_stream[m_pos++] & 0xFF);

    return m_pos;
}

u32 MemoryStream::read(s32& value) const
{
    ASSERT(m_length - m_pos >= sizeof(s32), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFF) << 24;
    value |= (m_stream[m_pos++] & 0xFF) << 16;
    value |= (m_stream[m_pos++] & 0xFF) << 8;
    value |= (m_stream[m_pos++] & 0xFF);

    return m_pos;
}

u32 MemoryStream::read(u64& value) const
{
    ASSERT(m_length - m_pos >= sizeof(u64), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFFLL) << 56;
    value |= (m_stream[m_pos++] & 0xFFLL) << 48;
    value |= (m_stream[m_pos++] & 0xFFLL) << 40;
    value |= (m_stream[m_pos++] & 0xFFLL) << 32;

    value |= (m_stream[m_pos++] & 0xFFLL) << 24;
    value |= (m_stream[m_pos++] & 0xFFLL) << 16;
    value |= (m_stream[m_pos++] & 0xFFLL) << 8;
    value |= (m_stream[m_pos++] & 0xFFLL);

    return m_pos;
}

u32 MemoryStream::read(s64& value) const
{
    ASSERT(m_length - m_pos >= sizeof(s64), "Invalid memory size");

    value = (m_stream[m_pos++] & 0xFFLL) << 56;
    value |= (m_stream[m_pos++] & 0xFFLL) << 48;
    value |= (m_stream[m_pos++] & 0xFFLL) << 40;
    value |= (m_stream[m_pos++] & 0xFFLL) << 32;

    value |= (m_stream[m_pos++] & 0xFFLL) << 24;
    value |= (m_stream[m_pos++] & 0xFFLL) << 16;
    value |= (m_stream[m_pos++] & 0xFFLL) << 8;
    value |= (m_stream[m_pos++] & 0xFFLL);

    return m_pos;
}

u32 MemoryStream::read(f32& value) const
{
    ASSERT(m_length - m_pos >= sizeof(f32), "Invalid memory size");

    s32& ival = *((s32*)&value);

    ival = (m_stream[m_pos++] & 0xFF) << 24;
    ival |= (m_stream[m_pos++] & 0xFF) << 16;
    ival |= (m_stream[m_pos++] & 0xFF) << 8;
    ival |= (m_stream[m_pos++] & 0xFF);

    return m_pos;
}

u32 MemoryStream::read(f64& value) const
{
    ASSERT(m_length - m_pos >= sizeof(f64), "Invalid memory size");

    s64& ival = *((s64*)&value);

    ival = (m_stream[m_pos++] & 0xFFLL) << 56;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 48;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 40;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 32;

    ival |= (m_stream[m_pos++] & 0xFFLL) << 24;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 16;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 8;
    ival |= (m_stream[m_pos++] & 0xFFLL);

    return m_pos;
}

u32 MemoryStream::read(f80& value) const
{
    ASSERT(m_length - m_pos >= sizeof(f80), "Invalid memory size");

    s64& ival = *((s64*)&value);

    ival = (m_stream[m_pos++] & 0xFFLL) << 56;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 48;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 40;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 32;

    ival |= (m_stream[m_pos++] & 0xFFLL) << 24;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 16;
    ival |= (m_stream[m_pos++] & 0xFFLL) << 8;
    ival |= (m_stream[m_pos++] & 0xFFLL);

    return m_pos;
}

u32 MemoryStream::read(bool& value) const
{
    ASSERT(m_length - m_pos >= sizeof(bool), "Invalid memory size");
    value = m_stream[m_pos++] != 0;

    return m_pos;
}

u32 MemoryStream::read(std::string& value) const
{
    u32 size = 0;
    read(size);

    if (size == 0)
    {
        return m_pos;
    }
    ASSERT(m_length - m_pos >= size, "Invalid memory size");

    value.resize(size);
    memcpy(&value[0], &m_stream[m_pos], size);

    m_pos += size;

    return m_pos;
}

u32 MemoryStream::write(const void* buffer, u32 size, u32 count)
{
    if (checkSize(size * count))
    {
        memcpy(m_stream + m_pos, buffer, size * count);
        m_pos += size * count;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(u8 value)
{
    if (checkSize(sizeof(u8)))
    {
        m_stream[m_pos++] = value;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(s8 value)
{
    if (checkSize(sizeof(s8)))
    {
        m_stream[m_pos++] = value;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(u16 value)
{
    if (checkSize(sizeof(u16)))
    {
        m_stream[m_pos++] = (value >> 8) & 0xFF;
        m_stream[m_pos++] = value & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(s16 value)
{
    if (checkSize(sizeof(s16)))
    {
        m_stream[m_pos++] = (value >> 8) & 0xFF;
        m_stream[m_pos++] = value & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(u32 value)
{
    if (checkSize(sizeof(u32)))
    {
        m_stream[m_pos++] = (value >> 24) & 0xFF;
        m_stream[m_pos++] = (value >> 16) & 0xFF;
        m_stream[m_pos++] = (value >> 8) & 0xFF;
        m_stream[m_pos++] = value & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(s32 value)
{
    if (checkSize(sizeof(s32)))
    {
        m_stream[m_pos++] = (value >> 24) & 0xFF;
        m_stream[m_pos++] = (value >> 16) & 0xFF;
        m_stream[m_pos++] = (value >> 8) & 0xFF;
        m_stream[m_pos++] = value & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(u64 value)
{
    if (checkSize(sizeof(u64)))
    {
        m_stream[m_pos++] = (value >> 56) & 0xFFLL;
        m_stream[m_pos++] = (value >> 48) & 0xFFLL;
        m_stream[m_pos++] = (value >> 40) & 0xFFLL;
        m_stream[m_pos++] = (value >> 32) & 0xFFLL;

        m_stream[m_pos++] = (value >> 24) & 0xFFLL;
        m_stream[m_pos++] = (value >> 16) & 0xFFLL;
        m_stream[m_pos++] = (value >> 8) & 0xFFLL;
        m_stream[m_pos++] = value & 0xFFLL;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(s64 value)
{
    if (checkSize(sizeof(s64)))
    {
        m_stream[m_pos++] = (value >> 56) & 0xFFLL;
        m_stream[m_pos++] = (value >> 48) & 0xFFLL;
        m_stream[m_pos++] = (value >> 40) & 0xFFLL;
        m_stream[m_pos++] = (value >> 32) & 0xFFLL;

        m_stream[m_pos++] = (value >> 24) & 0xFFLL;
        m_stream[m_pos++] = (value >> 16) & 0xFFLL;
        m_stream[m_pos++] = (value >> 8) & 0xFFLL;
        m_stream[m_pos++] = value & 0xFFLL;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(f32 value)
{
    if (checkSize(sizeof(f32)))
    {
        s32& ival = *((s32*)&value);

        m_stream[m_pos++] = (ival >> 24) & 0xFF;
        m_stream[m_pos++] = (ival >> 16) & 0xFF;
        m_stream[m_pos++] = (ival >> 8) & 0xFF;
        m_stream[m_pos++] = ival & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(f64 value)
{
    if (checkSize(sizeof(f64)))
    {
        s64& ival = *((s64*)&value);

        m_stream[m_pos++] = (ival >> 56) & 0xFFLL;
        m_stream[m_pos++] = (ival >> 48) & 0xFFLL;
        m_stream[m_pos++] = (ival >> 40) & 0xFFLL;
        m_stream[m_pos++] = (ival >> 32) & 0xFFLL;

        m_stream[m_pos++] = (ival >> 24) & 0xFFLL;
        m_stream[m_pos++] = (ival >> 16) & 0xFFLL;
        m_stream[m_pos++] = (ival >> 8) & 0xFFLL;
        m_stream[m_pos++] = ival & 0xFFLL;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(f80 value)
{
    if (checkSize(sizeof(f80)))
    {
        s32& ival = *((s32*)&value);

        m_stream[m_pos++] = (ival >> 24) & 0xFF;
        m_stream[m_pos++] = (ival >> 16) & 0xFF;
        m_stream[m_pos++] = (ival >> 8) & 0xFF;
        m_stream[m_pos++] = ival & 0xFF;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(bool value)
{
    if (checkSize(sizeof(bool)))
    {
        m_stream[m_pos++] = value;

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

u32 MemoryStream::write(const std::string value)
{
    if (checkSize(sizeof(u32)))
    {
        MemoryStream::write(static_cast<u32>(value.size()));
    }

    if (value.empty())
    {
        return m_pos;
    }

    if (checkSize(static_cast<u32>(value.size())))
    {
        memcpy(m_stream + m_pos, &value[0], value.size());
        m_pos += static_cast<u32>(value.size());

        if (m_pos > m_length)
        {
            m_length = m_pos;
        }
    }

    return m_pos;
}

void MemoryStream::seekBeg(u32 offset) const
{
    ASSERT(offset <= m_length, "Invalid memory size");
    m_pos = offset;
}

void MemoryStream::seekEnd(u32 offset) const
{
    ASSERT(offset <= m_length, "Invalid memory size");
    m_pos = m_length + offset;
}

void MemoryStream::seekCur(u32 offset) const
{
    ASSERT(offset <= m_length, "Invalid memory size");
    m_pos += offset;
}

u32 MemoryStream::tell() const
{
    return m_pos;
}

u32 MemoryStream::size() const
{
    return m_length;
}

u8* MemoryStream::map(u32 size) const
{
    u8* res = 0;

    ASSERT(size > 0 && m_pos + size <= m_length, "Invalid memory size");

    if (m_stream)
    {
        res = &m_stream[m_pos];
    }

    ASSERT(!m_mapped, "Memory not mapped");
    m_mapped = true;

    return res;
}

void MemoryStream::unmap() const
{
    ASSERT(m_mapped, "Memory mapped");
    m_mapped = false;
}

bool MemoryStream::isMapped() const
{
    return m_mapped;
}

u8* MemoryStream::getData() const
{
    ASSERT(!m_mapped, "Memory not mapped");
    return m_stream;
}

void MemoryStream::clear()
{
    if (m_stream)
    {
        if (m_allocator)
        {
            m_allocator->freeMemory(m_stream);
        }
        else
        {
            delete[] m_stream;
        }
        m_stream = nullptr;
    }

    m_length = 0;
    m_allocated = 0;
    m_pos = 0;
}

void MemoryStream::allocate(u32 size)
{
    if (m_stream)
    {
        if (m_allocator)
        {
            m_allocator->freeMemory(m_stream);
        }
        else
        {
            delete[] m_stream;
        }
        m_stream = nullptr;
    }

    m_allocated = size;
    m_length = size;
    if (m_allocator)
    {
        m_stream = (u8*)m_allocator->getMemory(m_allocated);
    }
    else
    {
        m_stream = new u8[m_allocated];
    }

    MemoryStream::seekBeg(0);
}

bool MemoryStream::checkSize(u32 size)
{
    if (m_allocated == 0)
    {
        MemoryStream::allocate(size);
        return true;
    }

    if (m_pos + size > m_allocated)
    {
        u8* oldStream = m_stream;
        s32 newAllocated = 2 * (m_pos + size);
        if (m_allocator)
        {
            m_stream = (u8*)m_allocator->getMemory(newAllocated);
        }
        else
        {
            m_stream = new u8[newAllocated];
        }

        memcpy(m_stream, oldStream, m_allocated);
        m_allocated = newAllocated;

        if (m_allocator)
        {
            m_allocator->freeMemory(oldStream);
        }
        else
        {
            delete[] oldStream;
        }
        oldStream = nullptr;
    }

    return true;
}

} //namespace stream
} //namespace v3d
