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
    * @brief Memory Heap struct. DirectX Render side
    */
    struct D3DMemoryHeap
    {
        ID3D12Heap* _heap;
        u32 _offset;

        bool operator==(const D3DMemoryHeap& otherHeap) const
        {
            if (this == &otherHeap)
            {
                return true;
            }

            return _heap == otherHeap._heap && _offset == otherHeap._offset;
        }

        bool operator!=(const D3DMemoryHeap& otherHeap) const
        {
            return !operator==(otherHeap);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Memory Heap class. DirectX Render side
    */
    class D3DHeapAllocator
    {
    public:

        D3DHeapAllocator(ID3D12Device* m_device) noexcept;
        virtual ~D3DHeapAllocator() = default;

        virtual D3DMemoryHeap allocate(u64 size, u64 align, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS heapFlags) = 0;
        virtual void deallocate(D3DMemoryHeap* heap) = 0;

    protected:

        friend class D3DMemory;

        ID3D12Device* const m_device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Memory manager. DirectX Render side
    */
    class D3DMemory final
    {
    public:

        static D3DMemoryHeap acquireHeap(D3DHeapAllocator& allocator, const D3D12_RESOURCE_DESC& desc, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS heapFlags);
        static void removeHeap(D3DHeapAllocator& allocator, D3DMemoryHeap& heap);

        static D3DMemoryHeap s_invalidMemory;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DSimpleHeapAllocator final : public D3DHeapAllocator
    {
    public:

        D3DSimpleHeapAllocator(ID3D12Device* device) noexcept;
        ~D3DSimpleHeapAllocator();

        D3DMemoryHeap allocate(u64 size, u64 align, const D3D12_HEAP_PROPERTIES& props, D3D12_HEAP_FLAGS heapFlags) override;
        void deallocate(D3DMemoryHeap* heap) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
