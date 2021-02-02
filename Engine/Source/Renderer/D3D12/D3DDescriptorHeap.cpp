#include "D3DDescriptorHeap.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "D3DDebug.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DDescriptorHeap::D3DDescriptorHeap(ID3D12DescriptorHeap* heap, D3D12_DESCRIPTOR_HEAP_DESC& desc) noexcept
    : m_heap(heap)
    , m_desc(desc)
    , m_increment(0)
{
    LOG_DEBUG("D3DDescriptorHeap::D3DDescriptorHeap constructor %llx", this);
}

D3DDescriptorHeap::~D3DDescriptorHeap()
{
    LOG_DEBUG("D3DDescriptorHeap::~D3DDescriptorHeap destructor %llx", this);
    SAFE_DELETE(m_heap);
}

D3D12_DESCRIPTOR_HEAP_DESC D3DDescriptorHeap::getDescription() const
{
    return m_desc;
}

ID3D12DescriptorHeap* D3DDescriptorHeap::getHandle() const
{
    return m_heap;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getCPUHandle() const
{
    ASSERT(m_heap, "nullptr");
    return m_heap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getGPUHandle() const
{
    ASSERT(m_heap, "nullptr");
    return m_heap->GetGPUDescriptorHandleForHeapStart();
}

u32 D3DDescriptorHeap::getIncrement() const
{
    return m_increment;
}


D3DDescriptorHeapManager::D3DDescriptorHeapManager(ID3D12Device* device) noexcept
    : m_device(device)
{
    LOG_DEBUG("D3DDescriptorHeapManager::D3DDescriptorHeapManager constructor %llx", this);
}

D3DDescriptorHeapManager::~D3DDescriptorHeapManager()
{
    LOG_DEBUG("D3DDescriptorHeapManager::D3DDescriptorHeapManager destructor %llx", this);
    freeDescriptorHeaps();
}

u32 D3DDescriptorHeapManager::getDescriptorNumSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        return 2048;

    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        return 512;

    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        return 2048;

    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
        return 128;

    default:
        ASSERT(false, "not found");
    }

    return 0;
}

D3DDescriptorHeap* D3DDescriptorHeapManager::allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = countDescriptors;
    heapDesc.Type = type;
    heapDesc.Flags = flags;
    heapDesc.NodeMask = 0;

    ID3D12DescriptorHeap* dxDescriptorHeap = nullptr;
    HRESULT result = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dxDescriptorHeap));
    if (FAILED(result))
    {
        LOG_ERROR("D3DDescriptorHeapManager::createDescriptorHeap: CreateDescriptorHeap is failed. Error: %s", D3DDebug::stringError(result).c_str());
        return nullptr;
    }
    ASSERT(dxDescriptorHeap, "nullptr");

    D3DDescriptorHeap* descriptorHeap = new D3DDescriptorHeap(dxDescriptorHeap, heapDesc);
    descriptorHeap->m_increment = m_device->GetDescriptorHandleIncrementSize(type);

    m_heapList[type].push_back(descriptorHeap);

    return descriptorHeap;
}

void D3DDescriptorHeapManager::deallocDescriptorHeap(D3DDescriptorHeap* heap)
{
    D3D12_DESCRIPTOR_HEAP_TYPE type = heap->m_desc.Type;
    auto found = std::find(m_heapList[type].begin(), m_heapList[type].end(), heap);
    ASSERT(found != m_heapList[type].end(), "not found");

    delete (*found);
    m_heapList[type].erase(found);
}

void D3DDescriptorHeapManager::freeDescriptorHeaps()
{
    for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
        for (auto heap : m_heapList[type])
        {
            delete heap;
        }
        m_heapList[type].clear();
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER


