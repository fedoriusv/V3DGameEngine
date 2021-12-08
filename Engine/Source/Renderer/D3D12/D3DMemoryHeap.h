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
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Memory Heap class. DirectX Render side
    */
    class D3DHeapAllocator
    {
    public:

        D3DHeapAllocator() noexcept = default;
        virtual ~D3DHeapAllocator() = default;

        virtual D3DMemoryHeap* allocate() = 0;
        virtual void deallocate(D3DMemoryHeap* heap) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Memory manager. DirectX Render side
    */
    class D3DMemory final
    {
    public:

        static D3DMemoryHeap acquireHeap(D3DHeapAllocator& allocator, const D3D12_HEAP_PROPERTIES& props);
        static void removeHeap(D3DHeapAllocator& allocator, D3DMemoryHeap* heap);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DSimpleHeapAllocator final : public D3DHeapAllocator
    {
    public:

        D3DSimpleHeapAllocator(ID3D12Device* device) noexcept;
        ~D3DSimpleHeapAllocator();

        D3DMemoryHeap* allocate() override;
        void deallocate(D3DMemoryHeap* heap) override;

    private:

        ID3D12Device* m_device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
