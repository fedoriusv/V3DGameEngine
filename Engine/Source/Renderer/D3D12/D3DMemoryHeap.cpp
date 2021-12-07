#include "D3DMemoryHeap.h"

#ifdef D3D_RENDER

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DMemoryHeapAllocator::D3DMemoryHeapAllocator(ID3D12Device* device) noexcept
    : m_device(device)
{
}

D3DMemoryHeapAllocator::~D3DMemoryHeapAllocator()
{
}

D3DMemoryHeap D3DMemoryHeapAllocator::acquireHeap()
{
    ID3D12Heap* heap = nullptr;
    u32 offset = 0;

    return {
        heap,
        offset
    };
}

ID3D12Heap* D3DMemoryHeapAllocator::createHeap(u64 size, u64 align)
{
    ID3D12Heap* heap;

    D3D12_HEAP_DESC desc = {};
    desc.SizeInBytes = core::alignUp(size, align);
    desc.Properties;
    desc.Alignment = align;
    desc.Flags = D3D12_HEAP_FLAG_NONE;

    HRESULT result = m_device->CreateHeap(&desc, DX_IID_PPV_ARGS(&heap));
    if (FAILED(result))
    {
        //
    }

    return nullptr;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER