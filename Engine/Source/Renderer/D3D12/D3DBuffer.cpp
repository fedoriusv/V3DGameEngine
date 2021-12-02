#include "D3DBuffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
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
    , m_resource(nullptr)

    , m_type(type)
    , m_size(size)
#if D3D_DEBUG
    , m_debugName(name)
#endif
{
    LOG_DEBUG("D3DBuffer::D3DBuffer constructor %llx", this);
}

D3DBuffer::~D3DBuffer()
{
    LOG_DEBUG("D3DBuffer::~D3DBuffer destructor %llx", this);
    ASSERT(!m_resource, "not nullptr");
}

bool D3DBuffer::create()
{
    if (m_resource)
    {
        ASSERT(false, "already created");
        return true;
    }

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);
    HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, DX_IID_PPV_ARGS(&m_resource));
    if (FAILED(result))
    {
        LOG_ERROR("D3DBuffer::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }
#if D3D_DEBUG
    wchar_t wtext[64];
    if (!m_debugName.empty())
    {
        mbstowcs(wtext, m_debugName.c_str(), m_debugName.size() + 1);
    }
    else
    {
        std::string debugName = "BufferResource: " + std::to_string(reinterpret_cast<const u64>(this));
        mbstowcs(wtext, debugName.c_str(), debugName.size() + 1);
    }
    m_resource->SetName(LPCWSTR(wtext));
#endif

    return true;
}

void D3DBuffer::destroy()
{
    SAFE_DELETE(m_resource);
}

u64 D3DBuffer::getSize() const
{
    return m_size;
}

bool D3DBuffer::upload(Context* context, u32 offset, u64 size, const void* data)
{
    if (!m_resource)
    {
        return false;
    }

    UINT8* bufferData = nullptr;
    CD3DX12_RANGE readRange = { offset, offset + size };
    HRESULT result = m_resource->Map(0, &readRange, reinterpret_cast<void**>(&bufferData));
    if (FAILED(result))
    {
        LOG_ERROR("D3DBuffer::upload map failed %s", D3DDebug::stringError(result).c_str());
        return false;
    }
    memcpy(bufferData, data, size);
    m_resource->Unmap(0, &readRange);

    return true;
}

ID3D12Resource* D3DBuffer::getResource() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource;
}

D3D12_GPU_VIRTUAL_ADDRESS D3DBuffer::getGPUAddress() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource->GetGPUVirtualAddress();
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
