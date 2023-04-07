#include "D3DConstantBuffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DBuffer.h"
#include "D3DDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DConstantBuffer::D3DConstantBuffer(ID3D12Device* device, u32 size, D3DHeapAllocator* allocator) noexcept
    : m_buffer(new D3DBuffer(device, Buffer::BufferType::ConstantBuffer, 0, size, "ConstantBuffer", allocator))
    , m_mappedData(nullptr)
{
    LOG_DEBUG("D3DConstantBuffer::D3DConstantBuffer constructor %llx", this);
}

D3DConstantBuffer::~D3DConstantBuffer()
{
    LOG_DEBUG("D3DConstantBuffer::~D3DConstantBuffer destructor %llx", this);
    ASSERT(!m_mappedData, "must be unmaped");
    if (m_buffer)
    {
        delete m_buffer;
        m_buffer = nullptr;
    }
}

bool D3DConstantBuffer::create()
{
    ASSERT(m_buffer, "nullptr");
    if (!m_buffer->create())
    {
        return false;
    }

    m_mappedData = m_buffer->map(0, m_buffer->getSize());
    ASSERT(m_mappedData, "can't to map");

    return true;
}

void D3DConstantBuffer::destroy()
{
    ASSERT(m_buffer, "nullptr");

    m_buffer->unmap(0, m_buffer->getSize());
    m_mappedData = nullptr;

    m_buffer->destroy();
}

void D3DConstantBuffer::update(u32 offset, u32 size, const void* data)
{
    ASSERT(m_buffer, "nullptr");
    ASSERT(m_mappedData, "not mapped");

    ASSERT(offset + size <= m_buffer->getSize(), "range out");
    memcpy(reinterpret_cast<u8*>(m_mappedData) + offset, data, size);
}


D3DConstantBufferManager::D3DConstantBufferManager(ID3D12Device* device, D3DHeapAllocator* allocator) noexcept
    : m_device(device)
    , m_allocator(allocator)

    , m_currentConstantBuffer({nullptr, 0})
{
    LOG_DEBUG("D3DConstantBufferManager::D3DConstantBufferManager constructor %llx", this);
}

D3DConstantBufferManager::~D3DConstantBufferManager()
{
    LOG_DEBUG("D3DConstantBufferManager::~D3DConstantBufferManager destructor %llx", this);
    ASSERT(m_usedConstantBuffers.empty(), "not empty");
    ASSERT(m_freeConstantBuffers.empty(), "not empty");
    ASSERT(!std::get<0>(m_currentConstantBuffer), "not nullptr");
}

std::tuple<D3DConstantBuffer*, u32> D3DConstantBufferManager::acquireConstanBuffer(u64 requestSize)
{
    u64 requestedSize = math::alignUp<u64>(requestSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    u32 desiredBufferSize = D3DDeviceCaps::getInstance()->memoryConstantBufferSize;
    if (std::get<0>(m_currentConstantBuffer))
    {
        u64 bufferSize = std::get<0>(m_currentConstantBuffer)->getSize();
        if (bufferSize < requestedSize)
        {
            LOG_WARNING("D3DConstantBufferManager::acquireConstanBuffer: Buffer size is less then requested. Size %d, Requested %d", bufferSize, requestedSize);
            desiredBufferSize = requestedSize;
        }

        if (requestedSize + std::get<1>(m_currentConstantBuffer) < bufferSize)
        {
            std::get<1>(m_currentConstantBuffer) += requestedSize;
            return m_currentConstantBuffer;
        }
        else
        {
            m_usedConstantBuffers.push_back(std::get<0>(m_currentConstantBuffer));
        }
    }

    if (!m_freeConstantBuffers.empty())
    {
        D3DConstantBuffer* buffer = m_freeConstantBuffers.front();
        if (buffer->getSize() < requestedSize)
        {
            LOG_WARNING("D3DConstantBufferManager::acquireConstanBuffer: Buffer size is less then requested. Size %d, Requested %d", buffer->getSize(), requestedSize);
            desiredBufferSize = requestedSize;
        }
        else
        {
            m_freeConstantBuffers.pop();
            m_currentConstantBuffer = { buffer, 0 };

            return m_currentConstantBuffer;
        }
    }

    ASSERT(requestedSize <= desiredBufferSize, "small size");
    D3DConstantBuffer* newBuffer = new D3DConstantBuffer(m_device, desiredBufferSize, m_allocator);
    if (!newBuffer->create())
    {
        LOG_ERROR("D3DConstantBufferManager::acquireConstanBuffer: create buffer is failed");
        delete newBuffer;

        return { nullptr, 0 };
    }

    m_currentConstantBuffer = { newBuffer, 0 };
    return m_currentConstantBuffer;
}

void D3DConstantBufferManager::updateConstantBufferStatus()
{
    if (std::get<0>(m_currentConstantBuffer))
    {
        m_usedConstantBuffers.push_back(std::get<0>(m_currentConstantBuffer));
        m_currentConstantBuffer = { nullptr, 0 };
    }

    for (auto iter  = m_usedConstantBuffers.begin(); iter != m_usedConstantBuffers.end();)
    {
        D3DConstantBuffer* buffer = (*iter);
        if (buffer->isUsed())
        {
            ++iter;
        }
        else
        {
            iter = m_usedConstantBuffers.erase(iter);
            m_freeConstantBuffers.push(buffer);
        }
    }
}

void D3DConstantBufferManager::destroyConstantBuffers()
{
    ASSERT(m_usedConstantBuffers.empty(), "not empty");
    ASSERT(!std::get<0>(m_currentConstantBuffer), "not nullptr");
    while(!m_freeConstantBuffers.empty())
    {
        auto cbuffer = m_freeConstantBuffers.front();
        m_freeConstantBuffers.pop();

        ASSERT(!cbuffer->isUsed(), "still used");
        cbuffer->destroy();
        delete cbuffer;
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER