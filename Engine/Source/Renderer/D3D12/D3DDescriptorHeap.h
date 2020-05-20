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
    * D3DDescriptorHeapHandle struct. DirectX Render side
    */
    struct D3DDescriptor : public D3DResource
    {
        D3DDescriptorHeap* _heap;
        u32 _offset;

        static CD3DX12_CPU_DESCRIPTOR_HANDLE createCPUDescriptorHandle(D3DDescriptor* desc);
        static CD3DX12_GPU_DESCRIPTOR_HANDLE createGPUDescriptorHandle(D3DDescriptor* desc);

        struct OffsetSort
        {
            bool operator()(const D3DDescriptor* desc0, const D3DDescriptor* desc1) const
            {
                return desc0->_offset < desc1->_offset;
            }
        };
    };

    /**
    * D3DDescriptorHeap class. DirectX Render side
    */
    class D3DDescriptorHeap final
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
        void* m_ptr;

        std::set<D3DDescriptor*, D3DDescriptor::OffsetSort> m_freeDescriptors;
        std::list<D3DDescriptor*> m_usedDescriptors;

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

        D3DDescriptor* acquireDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        bool freeDescriptor(D3DDescriptor* desc);

        D3DDescriptorHeap* allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        void deallocDescriptorHeap(D3DDescriptorHeap* heap);

        void freeDescriptorHeaps();

    private:

        static u32 getDescriptorNumSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

        D3DDescriptor* getFreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

        ID3D12Device* const m_device;
        std::vector<D3DDescriptorHeap*> m_heapList[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER