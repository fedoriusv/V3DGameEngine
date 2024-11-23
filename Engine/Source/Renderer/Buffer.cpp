#include "Buffer.h"
#include "Device.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

Buffer::Buffer(BufferUsageFlags usage) noexcept
    : m_buffer(nullptr)
    , m_usage(usage)
{
}

Buffer::~Buffer()
{
}


VertexBuffer::VertexBuffer(Device* device, BufferUsageFlags usage, u32 count, u32 size, const std::string& name) noexcept
    : Buffer(usage)
    , m_device(device)
    , m_size(size)
    , m_count(count)
{
    LOG_DEBUG("VertexBuffer::VertexBuffer constructor %llx", this);

    m_buffer = m_device->createBuffer(RenderBuffer::Type::VertexBuffer, usage, size, name);
    ASSERT(m_buffer.isValid(), "nullptr");
}

VertexBuffer::~VertexBuffer()
{
    LOG_DEBUG("VertexBuffer::VertexBuffer destructor %llx", this);

    m_device->destroyBuffer(m_buffer);
    m_buffer = BufferHandle(nullptr);
}


IndexBuffer::IndexBuffer(Device* device, BufferUsageFlags usage, IndexBufferType type, u32 count, const std::string& name) noexcept
    : Buffer(usage)
    , m_device(device)
    , m_type(type)
    , m_count(count)
{
    LOG_DEBUG("IndexBuffer::IndexBuffer constructor %llx", this);
    u64 size = m_count * (type == IndexBufferType::IndexType_16) ? sizeof(u16) : sizeof(u32);
    m_buffer = m_device->createBuffer(RenderBuffer::Type::IndexBuffer, usage, size * count, name);
    ASSERT(m_buffer.isValid(), "nullptr");
}

IndexBuffer::~IndexBuffer()
{
    LOG_DEBUG("IndexBuffer::IndexBuffer destructor %llx", this);

    m_device->destroyBuffer(m_buffer);
    m_buffer = BufferHandle(nullptr);
}

} //namespace renderer
} //namespace v3d