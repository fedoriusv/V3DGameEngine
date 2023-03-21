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

std::tuple<D3DBuffer*, u32> D3DConstantBufferManager::acquireConstanBuffer(u64 requestSize)
{
    u64 requestedSize = core::alignUp<u64>(requestSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

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
        D3DBuffer* buffer = m_freeConstantBuffers.front();
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
    D3DBuffer* newBuffer = new D3DBuffer(m_device, Buffer::BufferType::BufferType_ConstantBuffer, 0, desiredBufferSize, "ConstantBuffer", m_allocator);
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
        D3DBuffer* buffer = (*iter);
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