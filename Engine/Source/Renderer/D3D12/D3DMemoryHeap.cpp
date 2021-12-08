#include "D3DMemoryHeap.h"

#ifdef D3D_RENDER

namespace v3d
{
namespace renderer
{
namespace dx3d
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DMemoryHeap D3DMemory::acquireHeap(D3DHeapAllocator& allocator, const D3D12_HEAP_PROPERTIES& props)
{
    return *allocator.allocate();
}

void D3DMemory::removeHeap(D3DHeapAllocator& allocator, D3DMemoryHeap* heap)
{
    allocator.deallocate(heap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DSimpleHeapAllocator::D3DSimpleHeapAllocator(ID3D12Device* device) noexcept
    : m_device(device)
{
}

D3DSimpleHeapAllocator::~D3DSimpleHeapAllocator()
{
}

D3DMemoryHeap* D3DSimpleHeapAllocator::allocate()
{
    ID3D12Heap* heap;

    D3D12_HEAP_DESC desc = {};
    desc.SizeInBytes = core::alignUp(0, 0);
    desc.Properties;
    desc.Alignment = 0;
    desc.Flags = D3D12_HEAP_FLAG_NONE;

    HRESULT result = m_device->CreateHeap(&desc, DX_IID_PPV_ARGS(&heap));
    if (FAILED(result))
    {
        //
    }

    return nullptr;
}

void D3DSimpleHeapAllocator::deallocate(D3DMemoryHeap* heap)
{
}


} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER