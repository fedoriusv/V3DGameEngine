#include "D3DDescriptorHeap.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
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

D3D12_CPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getCPUHandle(u32 offset) const
{
    ASSERT(m_heap, "nullptr");
    return D3D12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart().ptr + m_increment * offset);
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

    for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
        ASSERT(std::get<0>(m_currentDescriptorHeap[type]) == nullptr, "must be nullptr");
        ASSERT(m_freeDescriptorHeaps[type].empty(), "must be empty");
        ASSERT(m_usedDescriptorHeaps[type].empty(), "must be empty");
#if D3D_DEBUG
        ASSERT(m_heapList[type].empty(), "must be empty");
#endif
    }
}

u32 D3DDescriptorHeapManager::getDescriptorNumSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        return 2048;

    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        return 2048;

    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        return 512;

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
    HRESULT result = m_device->CreateDescriptorHeap(&heapDesc, DX_IID_PPV_ARGS(&dxDescriptorHeap));
    if (FAILED(result))
    {
        LOG_ERROR("D3DDescriptorHeapManager::createDescriptorHeap: CreateDescriptorHeap is failed. Error: %s", D3DDebug::stringError(result).c_str());
        return nullptr;
    }
    ASSERT(dxDescriptorHeap, "nullptr");

    D3DDescriptorHeap* descriptorHeap = new D3DDescriptorHeap(dxDescriptorHeap, heapDesc);
    descriptorHeap->m_increment = m_device->GetDescriptorHandleIncrementSize(type);
#if D3D_DEBUG
    m_heapList[type].push_back(descriptorHeap);
#endif
    return descriptorHeap;
}

void D3DDescriptorHeapManager::deallocDescriptorHeap(D3DDescriptorHeap* heap)
{
    ASSERT(heap, "nullptr");
#if D3D_DEBUG
    D3D12_DESCRIPTOR_HEAP_TYPE type = heap->getDescription().Type;
    auto found = std::find(m_heapList[type].begin(), m_heapList[type].end(), heap);
    ASSERT(found != m_heapList[type].end(), "not found");
    m_heapList[heap->getDescription().Type].erase(found);
#endif

    ASSERT(heap->isUsed(), "must be free");
    delete heap;
}

void D3DDescriptorHeapManager::freeDescriptorHeaps()
{
    for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
#if D3D_DEBUG
        for (auto& heap : m_heapList[type])
        {
            ASSERT(!heap->isUsed(), "must be free");
        }
        m_heapList[type].clear();
#endif

        while (!m_freeDescriptorHeaps[type].empty())
        {
            auto& heap = m_freeDescriptorHeaps[type].front();
            ASSERT(!heap->isUsed(), "must be free");
            m_freeDescriptorHeaps->pop();
            delete heap;
        }
    }
}

std::tuple<D3DDescriptorHeap*, u32> D3DDescriptorHeapManager::getDescriptor(const DescriptorInfo& info)
{
    auto found = m_descriptors.find(info);
    if (found != m_descriptors.end())
    {
        return found->second;
    }

    return { nullptr, 0};
}

bool D3DDescriptorHeapManager::addDescriptor(const DescriptorInfo& info, const std::tuple<D3DDescriptorHeap*, u32>& heap)
{
    auto added = m_descriptors.emplace(info, heap);
    ASSERT(added.second, "can't to add");

    return added.second;
}

std::tuple<D3DDescriptorHeap*, u32> D3DDescriptorHeapManager::acquireDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors)
{
    ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "wrong heap type");
    D3DDescriptorHeap* currentHeap = std::get<0>(m_currentDescriptorHeap[type]);
    if (currentHeap)
    {
        if (countDescriptors <= std::get<2>(m_currentDescriptorHeap[type]))
        {
            u32 offset = std::get<1>(m_currentDescriptorHeap[type]) + countDescriptors;
            std::get<2>(m_currentDescriptorHeap[type]) -= countDescriptors;
            return { currentHeap, offset };
        }
        else
        {
            m_usedDescriptorHeaps[type].push_back(currentHeap);
        }
    }

    if (!m_freeDescriptorHeaps[type].empty())
    {
        currentHeap = m_freeDescriptorHeaps[type].front();
        m_freeDescriptorHeaps[type].pop();

        m_currentDescriptorHeap[type] = { currentHeap, 0, currentHeap->getDescription().NumDescriptors };
        return { currentHeap, 0 };
    }

    currentHeap = D3DDescriptorHeapManager::allocateDescriptorHeap(type, D3DDescriptorHeapManager::getDescriptorNumSize(type), D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    ASSERT(currentHeap, "nullptr");

    m_currentDescriptorHeap[type] = { currentHeap, 0, countDescriptors };
    return { currentHeap, 0 };
}

void D3DDescriptorHeapManager::updateDescriptorHeaps()
{
    for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;

        if (std::get<0>(m_currentDescriptorHeap[type]))
        {
            m_usedDescriptorHeaps[type].push_back(std::get<0>(m_currentDescriptorHeap[type]));
            m_currentDescriptorHeap[type] = {};
        }

        for (auto heapIter = m_usedDescriptorHeaps[type].begin(); heapIter != m_usedDescriptorHeaps[type].end();)
        {
            D3DDescriptorHeap* heap = (*heapIter);
            if (!heap->isUsed())
            {
                heapIter = m_usedDescriptorHeaps[type].erase(heapIter);
                m_freeDescriptorHeaps[type].push(heap);
                continue;
            }
            ++heapIter;
        }
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER


