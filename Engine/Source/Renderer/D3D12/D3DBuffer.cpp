#include "D3DBuffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "d3dx12.h"
#include "D3DDebug.h"
#include "D3DGraphicContext.h"
#include "D3DCommandList.h"
#include "D3DCommandListManager.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DBuffer::D3DBuffer(ID3D12Device* device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name) noexcept
    : m_device(device)

    , m_bufferResource(nullptr)

    , m_type(type)
    , m_size(size)
{
    LOG_DEBUG("D3DBuffer::D3DBuffer constructor %llx", this);
}

D3DBuffer::~D3DBuffer()
{
    LOG_DEBUG("D3DBuffer::~D3DBuffer destructor %llx", this);
    ASSERT(!m_bufferResource, "not nullptr");
}

bool D3DBuffer::create()
{
    if (m_bufferResource)
    {
        ASSERT(false, "already created");
        return true;
    }

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);
    HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_bufferResource));
    if (FAILED(result))
    {
        LOG_ERROR("D3DBuffer::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}

void D3DBuffer::destroy()
{
    SAFE_DELETE(m_bufferResource);
}

u64 D3DBuffer::getSize() const
{
    return m_size;
}

bool D3DBuffer::upload(Context* context, u32 offset, u64 size, const void* data)
{
    if (!m_bufferResource)
    {
        return false;
    }

    UINT8* bufferData = nullptr;
    CD3DX12_RANGE readRange = { offset, offset + size };
    HRESULT result = m_bufferResource->Map(0, &readRange, reinterpret_cast<void**>(&bufferData));
    if (FAILED(result))
    {
        LOG_ERROR("D3DBuffer::upload map failed %s", D3DDebug::stringError(result).c_str());
        return false;
    }
    memcpy(bufferData, data, size);
    m_bufferResource->Unmap(0, &readRange);

    return true;
}

const CD3DX12_CPU_DESCRIPTOR_HANDLE& D3DBuffer::getDescriptorHandle() const
{
    return m_handle;
}

ID3D12Resource* D3DBuffer::getResource() const
{
    ASSERT(m_bufferResource, "nullptr");
    return m_bufferResource;
}

D3D12_GPU_VIRTUAL_ADDRESS D3DBuffer::getGPUAddress() const
{
    ASSERT(m_bufferResource, "nullptr");
    return m_bufferResource->GetGPUVirtualAddress();
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
