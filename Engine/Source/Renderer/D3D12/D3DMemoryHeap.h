#pragma once

#include "Common.h"
#include "Utils/Singleton.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Memory Heap struct. DirectX Render side
    */
    struct D3DMemoryHeap
    {
        ID3D12Heap* _heap;
        u32 _offset;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Memory Heap manager. DirectX Render side
    */
    class D3DMemoryHeapAllocator final
    {
    public:

        explicit D3DMemoryHeapAllocator(ID3D12Device* device) noexcept;
        ~D3DMemoryHeapAllocator();

        D3DMemoryHeap acquireHeap();

    private:

        ID3D12Heap* createHeap(u64 size, u64 align);

        ID3D12Device* const m_device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
