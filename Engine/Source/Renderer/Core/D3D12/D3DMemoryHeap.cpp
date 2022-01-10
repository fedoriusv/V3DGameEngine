#include "D3DMemoryHeap.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DHeapAllocator::D3DHeapAllocator(ID3D12Device* device) noexcept
    : m_device(device)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DMemoryHeap D3DMemory::s_invalidMemory =
{
    nullptr,
    0
};

D3DMemoryHeap D3DMemory::acquireHeap(D3DHeapAllocator& allocator, const D3D12_RESOURCE_DESC& desc, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS heapFlags)
{
    D3D12_RESOURCE_ALLOCATION_INFO info = allocator.m_device->GetResourceAllocationInfo(0, 1, &desc);

    return allocator.allocate(info.SizeInBytes, info.Alignment, props, heapFlags);
}

void D3DMemory::removeHeap(D3DHeapAllocator& allocator, D3DMemoryHeap& heap)
{
    if (heap != D3DMemory::s_invalidMemory)
    {
        allocator.deallocate(&heap);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DSimpleHeapAllocator::D3DSimpleHeapAllocator(ID3D12Device* device) noexcept
    : D3DHeapAllocator(device)
{
}

D3DSimpleHeapAllocator::~D3DSimpleHeapAllocator()
{
}

D3DMemoryHeap D3DSimpleHeapAllocator::allocate(u64 size, u64 align, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS heapFlags)
{
    ID3D12Heap* heap = nullptr;

    ASSERT(size == core::alignUp(size, align), "must be aligned");
    D3D12_HEAP_DESC desc = {};
    desc.SizeInBytes = size;
    desc.Alignment = align;
    desc.Properties = props;
    desc.Flags = heapFlags;

    HRESULT result = m_device->CreateHeap(&desc, DX_IID_PPV_ARGS(&heap));
    if (FAILED(result))
    {
        LOG_ERROR("D3DSimpleHeapAllocator::allocate CreateHeap is failed. Error %s", D3DDebug::stringError(result).c_str());
        return D3DMemory::s_invalidMemory;
    }

    return {
        heap,
        0
    };
}

void D3DSimpleHeapAllocator::deallocate(D3DMemoryHeap* heap)
{
    SAFE_DELETE(heap->_heap);
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER