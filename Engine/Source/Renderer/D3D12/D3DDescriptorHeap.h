#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DDescriptorHeapManager;
    class D3DDescriptorHeap;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DDescriptorHeap class. DirectX Render side
    */
    class D3DDescriptorHeap final : public D3DResource
    {
    public:

        explicit D3DDescriptorHeap(ID3D12DescriptorHeap* heap, D3D12_DESCRIPTOR_HEAP_DESC& desc) noexcept;
        ~D3DDescriptorHeap();

        D3D12_DESCRIPTOR_HEAP_DESC getDescription() const;
        ID3D12DescriptorHeap* getHandle() const;

        D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle() const;
        D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const;

        u32 getIncrement() const;

    private:

        ID3D12DescriptorHeap* m_heap;
        D3D12_DESCRIPTOR_HEAP_DESC m_desc;
        u32 m_increment;

        friend D3DDescriptorHeapManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /**
    * D3DDescriptorHeapManager final class. DirectX Render side
    */
    class D3DDescriptorHeapManager final
    {
    public:

        explicit D3DDescriptorHeapManager(ID3D12Device* device) noexcept;
        ~D3DDescriptorHeapManager();

        D3DDescriptorHeap* allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        void deallocDescriptorHeap(D3DDescriptorHeap* heap);

        void freeDescriptorHeaps();

    private:

        static u32 getDescriptorNumSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

        ID3D12Device* const m_device;
        std::vector<D3DDescriptorHeap*> m_heapList[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER