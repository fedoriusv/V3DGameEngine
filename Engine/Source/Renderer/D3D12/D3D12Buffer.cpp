#include "D3D12Buffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include "D3D12Debug.h"
#   include "D3D12GraphicContext.h"
#   include "D3D12CommandList.h"
#   include "D3D12CommandListManager.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3DBuffer::D3DBuffer(ID3D12Device* device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name) noexcept
    : m_device(device)

    , m_bufferResource(nullptr)

    , m_type(type)
    , m_size(size)
{
}

D3DBuffer::~D3DBuffer()
{
    ASSERT(!m_bufferResource, "not nullptr");
}

bool D3DBuffer::create()
{
    //if (m_bufferResource)
    //{
    //    return true;
    //}

    //D3D12_HEAP_PROPERTIES heapProperties = {};

    //D3D12_RESOURCE_DESC resourceDesc = {};
    //resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //resourceDesc.Alignment = 0;
    //resourceDesc.Width = 1;
    //resourceDesc.Height = 1;
    //resourceDesc.DepthOrArraySize = 1;
    //resourceDesc.MipLevels = 1;
    //resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    //resourceDesc.SampleDesc;
    //resourceDesc.Layout;
    //resourceDesc.Flags;

    //CD3DX12_RESOURCE_DESC::Buffer();
    ////TODO
    //HRESULT result = m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_bufferResource));
    //if (FAILED(result))
    //{
    //    LOG_ERROR("D3DBuffer::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
    //    return false;
    //}

    return true;
}

void D3DBuffer::destroy()
{
}

bool D3DBuffer::upload(Context* context, u32 offset, u64 size, void* data)
{
    return false;
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

} //namespace d3d12
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER
