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

    class D3DImage;
    class D3DSampler;
    class D3DBuffer;
    class D3DGraphicsCommandList;
    class D3DGraphicPipelineState;
    class D3DDescriptorHeapManager;
    class D3DDescriptorHeap;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DDescriptorSetState final
    {
    public:

        D3DDescriptorSetState(ID3D12Device* device, D3DDescriptorHeapManager* manager) noexcept;

        D3DDescriptorSetState(const D3DDescriptorSetState&) = delete;
        ~D3DDescriptorSetState();

        static D3D12_DESCRIPTOR_HEAP_TYPE convertDescriptorTypeToHeapType(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType);

        template<class TResource, class ... Args>
        bool bindDescriptor(u32 space, u32 binding, const TResource* resource, Args ...args)
        {
            static_assert(std::is_base_of<D3DResource, TResource>(), "D3DDescriptorSetState::bindDescriptor wrong type");

            Binding bind;
            bind._space = space;
            bind._binding = binding;
            bind._array = 1;

            if constexpr (std::is_same<TResource, D3DImage>())
            {
                bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                bind._resource._image = { resource };
            }
            else if constexpr (std::is_same<TResource, D3DBuffer>())
            {
                auto function = [&bind](const D3DBuffer* buffer, u32 offset, u32 size) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    bind._resource._constantBuffer = { buffer, offset, size };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DSampler>())
            {
                bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                bind._resource._sampler = { resource };
            }
            else
            {
                ASSERT(false, "unsupported resource");
                return false;
            }

            auto iter = m_descriptorSetTable.insert(bind);
            if (!iter.second)
            {
                ASSERT(false, "binding is already presentd inside table");
                return false;
            }

            return true;
        }

        bool updateDescriptorSets(D3DGraphicsCommandList* cmdList, D3DGraphicPipelineState* pipeline);
        void updateStatus();
        void invalidateDescriptorSetTable();

    private:

        struct Binding
        {
            struct Less
            {
                bool operator()(const Binding& b0, const Binding& b1) const
                {
                    if (b0._space == b1._space)
                    {
                        return b0._binding < b1._binding;
                    }

                    return b0._space < b1._space;
                }
            };

            u32 _space;
            u32 _binding;
            u32 _array;
            D3D12_DESCRIPTOR_RANGE_TYPE _type;
            union
            {
                struct ShaderResource
                {
                    const D3DImage* _image;
                };

                struct ConstntBuffer
                {
                    const D3DBuffer* _buffer;
                    u32 _offset;
                    u32 _size;
                };

                struct Sampler
                {
                    const D3DSampler* _sampler;
                };

                ShaderResource _image;
                ConstntBuffer _constantBuffer;
                Sampler _sampler;
            }
            _resource;
        };

        using DescriptorTableContainer = std::map<u32, std::set<Binding, Binding::Less>>;
        void composeDescriptorSetTable(const D3DGraphicPipelineState* pipeline, DescriptorTableContainer table[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);

        void updateDescriptorTable(D3DDescriptorHeap* heap, const DescriptorTableContainer& table, D3D12_DESCRIPTOR_HEAP_TYPE type, std::map<u32, std::tuple<D3DDescriptorHeap*, u32>>& descTable);

        ID3D12Device* m_device;
        D3DDescriptorHeapManager* m_heapManager;

        std::set<Binding, Binding::Less> m_descriptorSetTable;
        std::list<D3DDescriptorHeap*> m_usedDescriptorHeaps;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER