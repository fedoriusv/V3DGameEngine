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

CD3DX12_CPU_DESCRIPTOR_HANDLE D3DDescriptor::createCPUDescriptorHandle(D3DDescriptor* desc)
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(desc->_heap->getCPUHandle(), desc->_offset, desc->_heap->getIncrement());
}

CD3DX12_GPU_DESCRIPTOR_HANDLE D3DDescriptor::createGPUDescriptorHandle(D3DDescriptor* desc)
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(desc->_heap->getGPUHandle(), desc->_offset, desc->_heap->getIncrement());
}

D3DDescriptorHeap::D3DDescriptorHeap(ID3D12DescriptorHeap* heap, D3D12_DESCRIPTOR_HEAP_DESC& desc) noexcept
    : m_heap(heap)
    , m_desc(desc)
    , m_increment(0)
    , m_ptr(nullptr)
{
    LOG_DEBUG("D3DDescriptorHeap::D3DDescriptorHeap constructor %llx", this);
}

D3DDescriptorHeap::~D3DDescriptorHeap()
{
    LOG_DEBUG("D3DDescriptorHeap::~D3DDescriptorHeap destructor %llx", this);
    ASSERT(m_usedDescriptors.empty(), "not empty");

    SAFE_DELETE(m_heap);

    if (m_ptr)
    {
        free(m_ptr);
        m_ptr = nullptr;
    }
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

D3DDescriptor* D3DDescriptorHeapManager::acquireDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3DDescriptor* descripror = getFreeDescriptor(type);
    if (!descripror)
    {
        D3DDescriptorHeap* newHeap = allocateDescriptorHeap(type, D3DDescriptorHeapManager::getDescriptorNumSize(type), flags);
        ASSERT(newHeap, "nullptr");
        m_heapList[type].push_back(newHeap);

        descripror = getFreeDescriptor(type);
    }

    ASSERT(descripror, "nullptr");
    ASSERT(descripror->_heap->getDescription().Flags == flags, "wrong flag");

    return descripror;
}

bool D3DDescriptorHeapManager::freeDescriptor(D3DDescriptor* desc)
{
    auto iter = std::find(desc->_heap->m_usedDescriptors.begin(), desc->_heap->m_usedDescriptors.end(), desc);
    if (iter == desc->_heap->m_usedDescriptors.end())
    {
        ASSERT(false, "wrong");
        return false;
    }

    desc->_heap->m_usedDescriptors.erase(iter);
    desc->_heap->m_freeDescriptors.insert(desc);

    return true;
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

    u8* heap = reinterpret_cast<u8*>(malloc(heapDesc.NumDescriptors * sizeof(D3DDescriptor)));

    D3DDescriptorHeap* descriptorHeap = new D3DDescriptorHeap(dxDescriptorHeap, heapDesc);
    descriptorHeap->m_increment = m_device->GetDescriptorHandleIncrementSize(type);
    descriptorHeap->m_ptr = heap;

    for (u32 i = 0; i < heapDesc.NumDescriptors; ++i)
    {
        D3DDescriptor* desc = new(reinterpret_cast<D3DDescriptor*>(heap + (i * sizeof(D3DDescriptor)))) D3DDescriptor();
        desc->_heap = descriptorHeap;
        desc->_offset = i;

        descriptorHeap->m_freeDescriptors.insert(desc);
    }

    return descriptorHeap;
}

void D3DDescriptorHeapManager::deallocDescriptorHeap(D3DDescriptorHeap* heap)
{
    ASSERT(heap->m_usedDescriptors.empty(), "still used");
    SAFE_DELETE(heap->m_heap);
}

D3DDescriptor* D3DDescriptorHeapManager::getFreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    for (auto heap : m_heapList[type])
    {

        if (!heap->m_freeDescriptors.empty())
        {
            D3DDescriptor* desc = *(heap->m_freeDescriptors.begin());
            heap->m_freeDescriptors.erase(heap->m_freeDescriptors.begin());

            heap->m_usedDescriptors.push_back(desc);

            return desc;
        }
    }

    return nullptr;
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


