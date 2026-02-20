#include "Buffer.h"
#include "Device.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

Buffer::Buffer(BufferUsageFlags usage) noexcept
    : m_buffer(nullptr)
    , m_usage(usage)
{
}

Buffer::~Buffer()
{
}

void* Buffer::map_Impl(u32 offset, u32 size)
{
    ASSERT(m_buffer.isValid(), "nullptr");
    ASSERT(Buffer::hasUsageFlag(BufferUsage::Buffer_GPUWriteCocherent) || Buffer::hasUsageFlag(BufferUsage::Buffer_GPUWriteCached) || Buffer::hasUsageFlag(BufferUsage::Buffer_GPURead), "must be dynamic");
    RenderBuffer* buffer = m_buffer.as<RenderBuffer>();
    return buffer->map(offset, size);
}

void Buffer::unmap(u32 offset, u32 size)
{
    ASSERT(m_buffer.isValid(), "nullptr");
    ASSERT(Buffer::hasUsageFlag(BufferUsage::Buffer_GPUWriteCocherent) || Buffer::hasUsageFlag(BufferUsage::Buffer_GPUWriteCached) || Buffer::hasUsageFlag(BufferUsage::Buffer_GPURead), "must be dynamic");
    RenderBuffer* buffer = m_buffer.as<RenderBuffer>();
    buffer->unmap(offset, size);
}

    /////////////////////////////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer(Device* device, BufferUsageFlags usage, u32 count, u32 size, const std::string& name) noexcept
    : Buffer(usage)
    , m_device(device)
    , m_size(size)
    , m_count(count)
{
    m_buffer = m_device->createBuffer(RenderBuffer::Type::VertexBuffer, usage, size, name);
    ASSERT(m_buffer.isValid(), "nullptr");
}

VertexBuffer::~VertexBuffer()
{
    m_device->destroyBuffer(m_buffer);
    m_buffer = BufferHandle(nullptr);
}

    /////////////////////////////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer(Device* device, BufferUsageFlags usage, IndexBufferType type, u32 count, const std::string& name) noexcept
    : Buffer(usage)
    , m_device(device)
    , m_type(type)
    , m_count(count)
{
    u64 size = m_count * (type == IndexBufferType::IndexType_16) ? sizeof(u16) : sizeof(u32);
    m_buffer = m_device->createBuffer(RenderBuffer::Type::IndexBuffer, usage, size * count, name);
    ASSERT(m_buffer.isValid(), "nullptr");
}

IndexBuffer::~IndexBuffer()
{
    m_device->destroyBuffer(m_buffer);
    m_buffer = BufferHandle(nullptr);
}

    /////////////////////////////////////////////////////////////////////////////////////////////////////

UnorderedAccessBuffer::UnorderedAccessBuffer(Device* device, BufferUsageFlags usage, u32 size, const std::string& name) noexcept
    : Buffer(usage)
    , m_device(device)
    , m_size(size)
{
    m_buffer = m_device->createBuffer(RenderBuffer::Type::UnorderedAccess, usage, size, name);
    ASSERT(m_buffer.isValid(), "nullptr");
}

UnorderedAccessBuffer::~UnorderedAccessBuffer()
{
    m_device->destroyBuffer(m_buffer);
    m_buffer = BufferHandle(nullptr);
}

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d