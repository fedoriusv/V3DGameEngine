#include "Buffer.h"

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


VertexBuffer::VertexBuffer(Device* device, BufferUsageFlags usage, u64 size, const std::string& name) noexcept
    : Buffer(usage)
    , m_size(size)
{
}

VertexBuffer::~VertexBuffer()
{
}


IndexBuffer::IndexBuffer(BufferUsageFlags usage, IndexBufferType type, u32 count, const std::string& name) noexcept
    : Buffer(usage)
    , m_type(type)
    , m_count(count)
{
}

IndexBuffer::~IndexBuffer()
{
}

} //namespace renderer
} //namespace v3d