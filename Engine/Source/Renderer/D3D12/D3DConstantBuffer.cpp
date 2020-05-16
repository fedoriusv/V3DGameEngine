#include "D3DConstantBuffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DBuffer.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DConstantBufferManager::D3DConstantBufferManager(ID3D12Device* device) noexcept
    : m_device(device)
{
    LOG_DEBUG("D3DConstantBufferManager::D3DConstantBufferManager constructor %llx", this);
}

D3DConstantBufferManager::~D3DConstantBufferManager()
{
    LOG_DEBUG("D3DConstantBufferManager::~D3DConstantBufferManager destructor %llx", this);
    ASSERT(m_usedConstantBuffers.empty(), "not empty");

    //TODO
    ASSERT(m_freeConstantBuffers.empty(), "not empty");

}

D3DBuffer* D3DConstantBufferManager::acquireConstanBuffer(u64 requestSize)
{
    u64 requestedSize = core::alignUp<u64>(requestSize, 256);

    if (!m_freeConstantBuffers.empty())
    {
        D3DBuffer* buffer = m_freeConstantBuffers.front();
        m_freeConstantBuffers.pop();
        ASSERT(buffer->getSize() >= requestedSize, "small size");

        m_usedConstantBuffers.push_back(buffer);
        return buffer;
    }

    ASSERT(requestedSize <= k_constantBufferSize, "small size");
    D3DBuffer* newBuffer = new D3DBuffer(m_device, Buffer::BufferType::BufferType_UniformBuffer, 0, k_constantBufferSize);
    if (!newBuffer->create())
    {
        LOG_ERROR("D3DConstantBufferManager::acquireConstanBuffer: create buffer is failed");
        delete newBuffer;

        return nullptr;
    }

    m_usedConstantBuffers.push_back(newBuffer);
    return newBuffer;
}

void D3DConstantBufferManager::updateStatus()
{
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

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER