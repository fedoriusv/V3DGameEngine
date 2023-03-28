#pragma once

#include "Common.h"
#include "Renderer/Core/Image.h"
#include "Renderer/ShaderProperties.h"

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
    class D3DImage;
    class D3DSampler;
    class D3DBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DBinding struct. DirectX Render side
    * Size 4 bytes
    */
    struct D3DBinding
    {
        u32 _space                          : 8  = 0;
        u32 _register                       : 15 = 0;
        u32 _array                          : 5  = 0;
        D3D12_DESCRIPTOR_RANGE_TYPE _type   : 3 = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        u32 _uav                            : 1 = 0;

        struct Less
        {
            bool operator()(const D3DBinding& b0, const D3DBinding& b1) const
            {
                static_assert(sizeof(D3DBinding) == 4, "Wrong size");
                return memcmp(&b0, &b1, sizeof(D3DBinding)) < 0;
            }
        };
    };

    /**
    * @brief D3DBindingResource struct. DirectX Render side
    * Size 16 bytes
    */
    struct D3DBindingResource
    {
        D3DBindingResource() noexcept
        {
            memset(this, 0, sizeof(D3DBindingResource));
            static_assert(sizeof(D3DBindingResource) == 16, "D3DBindingResource wrong size");
        }

        union
        {
            struct ImageResource
            {
                const D3DImage* _image;
                Image::Subresource _subresource;
            };

            struct BufferResource
            {
                const D3DBuffer* _buffer;
                u32 _offset;
                u32 _size;
            };

            struct Sampler
            {
                const D3DSampler* _sampler;
                u64 _padding;
            };

            ImageResource _image;
            BufferResource _constantBuffer;
            Sampler _sampler;
            ImageResource _UAVImage;
            BufferResource _UAVBuffer;
        }
        _resource;
    };


    using DescriptorTableContainer = std::tuple<const std::vector<D3DBinding>, const std::vector<D3DBindingResource>>;
    using DescriptorTableContainerRef = std::tuple<const std::vector<D3DBinding>&, const std::array<D3DBindingResource, k_maxDescriptorBindingCount>&>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DDescriptorHeap class. DirectX Render side
    */
    class D3DDescriptorHeap final : public D3DResource
    {
    public:

        static D3D12_DESCRIPTOR_HEAP_TYPE convertDescriptorTypeToHeapType(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType);

        explicit D3DDescriptorHeap(ID3D12DescriptorHeap* heap, D3D12_DESCRIPTOR_HEAP_DESC& desc) noexcept;
        ~D3DDescriptorHeap();

        const D3D12_DESCRIPTOR_HEAP_DESC& getDescription() const;
        ID3D12DescriptorHeap* getHandle() const;

        D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle() const;
        D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(u32 offset) const;

        D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const;
        D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(u32 offset) const;

        u32 getIncrement() const;

    private:

        ID3D12DescriptorHeap* m_heap;
        D3D12_DESCRIPTOR_HEAP_DESC m_desc;
        u32 m_increment;

        friend D3DDescriptorHeapManager;
    };

    inline const D3D12_DESCRIPTOR_HEAP_DESC& D3DDescriptorHeap::getDescription() const
    {
        return m_desc;
    }

    inline ID3D12DescriptorHeap* D3DDescriptorHeap::getHandle() const
    {
        ASSERT(m_heap, "nullptr");
        return m_heap;
    }

    inline D3D12_CPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getCPUHandle() const
    {
        ASSERT(m_heap, "nullptr");
        return m_heap->GetCPUDescriptorHandleForHeapStart();
    }

    inline D3D12_CPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getCPUHandle(u32 offset) const
    {
        ASSERT(m_heap, "nullptr");
        return D3D12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart().ptr + m_increment * offset);
    }

    inline D3D12_GPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getGPUHandle() const
    {
        ASSERT(m_heap, "nullptr");
        return m_heap->GetGPUDescriptorHandleForHeapStart();
    }

    inline D3D12_GPU_DESCRIPTOR_HANDLE D3DDescriptorHeap::getGPUHandle(u32 offset) const
    {
        ASSERT(m_heap, "nullptr");
        return D3D12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart().ptr + m_increment * offset);
    }

    inline u32 D3DDescriptorHeap::getIncrement() const
    {
        return m_increment;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DDescriptorHeapManager final class. DirectX Render side
    */
    class D3DDescriptorHeapManager final
    {
    public:

        struct DescriptorTableInfo
        {
            struct Hash
            {
                u64 operator()(const DescriptorTableInfo& key) const
                {
                    ASSERT(key._key != 0, "must be valid");
                    return key._key;
                }
            };

            struct Compare
            {
                bool operator()(const DescriptorTableInfo& info0, const DescriptorTableInfo& info1) const
                {
                    if (info0._key != info1._key)
                    {
                        return false;
                    }

                    /*if (info0._bindings.size() != info1._bindings.size())
                    {
                        return false;
                    }*/

                    return /*memcmp(info0._bindings.data(), info1._bindings.data(), sizeof(D3DBinding) * info0._bindings.size()) == 0 &&*/
                        memcmp(info0._resources.data(), info1._resources.data(), sizeof(D3DBindingResource) * info0._resources.size()) == 0;
                }
            };

            explicit DescriptorTableInfo(const std::vector<D3DBinding>& bindings, const std::array<D3DBindingResource, k_maxDescriptorBindingCount>& resources, u32 hash) noexcept
                : _key(hash)
                /*, _bindings(bindings)*/
                , _resources(resources)
            {
            }

            u64 _key = 0;
            /*const std::vector<D3DBinding> _bindings;*/
            const std::array<D3DBindingResource, k_maxDescriptorBindingCount> _resources;
        };

        static u32 getDescriptorNumSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

        explicit D3DDescriptorHeapManager(ID3D12Device* device) noexcept;
        ~D3DDescriptorHeapManager();

        std::tuple<D3DDescriptorHeap*, u32> getDescriptor(const DescriptorTableInfo& info);
        bool addDescriptor(const DescriptorTableInfo& info, const std::tuple<D3DDescriptorHeap*, u32>& heap);

        std::tuple<D3DDescriptorHeap*, u32> acquireDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors);
        void freeDescriptorHeaps();

        void updateDescriptorHeaps();

        D3DDescriptorHeap* allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 countDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        void deallocDescriptorHeap(D3DDescriptorHeap* heap);

    private:

        ID3D12Device* const m_device;

        std::queue<D3DDescriptorHeap*> m_freeDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        std::list<D3DDescriptorHeap*> m_usedDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        std::array<std::tuple<D3DDescriptorHeap*, u32, u32>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_currentDescriptorHeap;

        std::unordered_map<DescriptorTableInfo, std::tuple<D3DDescriptorHeap*, u32>, DescriptorTableInfo::Hash, DescriptorTableInfo::Compare> m_descriptors;

#if DEBUG_OBJECT_MEMORY
        std::vector<D3DDescriptorHeap*> m_heapList[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
#endif //DEBUG_OBJECT_MEMORY
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER