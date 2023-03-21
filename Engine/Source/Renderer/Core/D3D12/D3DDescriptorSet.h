#pragma once

#include "Common.h"
#include "Renderer/Core/Image.h"
#include "Renderer/ShaderProperties.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"
#include "D3DDescriptorHeap.h"

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
    class D3DPipelineState;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DDescriptorSetState final class. DirectX Render side
    */
    class D3DDescriptorSetState final
    {
    public:

        explicit D3DDescriptorSetState(ID3D12Device* device, D3DDescriptorHeapManager* manager) noexcept;
        ~D3DDescriptorSetState();

        static D3D12_DESCRIPTOR_HEAP_TYPE convertDescriptorTypeToHeapType(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType);

        template<class TResource, bool UAV, class ... Args>
        bool bindDescriptor(u32 space, u32 reg, u32 array, const TResource* resource, Args ...args)
        {
            static_assert(std::is_base_of<D3DResource, TResource>(), "D3DDescriptorSetState::bindDescriptor wrong type");
            ASSERT(space < k_maxDescriptorSetCount, "Space range out index");

            D3DBinding bind;
            bind._register = reg;
            bind._space = space;
            bind._array = array;
 
            D3DBindingResource bindingResource;
            if constexpr (std::is_same<TResource, D3DImage>() && UAV == false)
            {
                auto function = [&bind, &bindingResource](const D3DImage* image, const Image::Subresource& subresource) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    bindingResource._resource._image = { image, subresource };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DBuffer>() && UAV == false)
            {
                auto function = [&bind, &bindingResource](const D3DBuffer* buffer, u32 offset, u32 size) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    bind._direct = true;
                    bindingResource._resource._constantBuffer = { buffer, offset, size };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DImage>() && UAV == true)
            {
                auto function = [&bind, &bindingResource](const D3DImage* image, const Image::Subresource& subresource) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    bindingResource._resource._UAV._image = { image, subresource };
                    bindingResource._resource._UAV._buffer = { nullptr, 0, 0 };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DBuffer>() && UAV == true)
            {
                auto function = [&bind, &bindingResource](const D3DBuffer* buffer, u32 offset, u32 size) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    bindingResource._resource._UAV._buffer = { resource, offset, size };
                    bindingResource._resource._UAV._image = nullptr;
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DSampler>())
            {
                bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                bindingResource._resource._sampler = { resource };
            }
            else
            {
                ASSERT(false, "unsupported resource");
                return false;
            }

            static_assert(sizeof(D3DBindingResource) == 32, "DBindingResource wrong size");
            auto iter = m_boundedDescriptorSets[space].emplace(bind, bindingResource);
            if (!iter.second)
            {
                ASSERT(false, "binding is already presentd inside table");
                return false;
            }

            return true;
        }

        bool updateDescriptorSets(D3DGraphicsCommandList* cmdList, D3DPipelineState* pipeline);
        void updateStatus(); //Call per submit/present
        void invalidateDescriptorSetTable(); //Call per draw

    private:

        D3DDescriptorSetState() = delete;
        D3DDescriptorSetState(const D3DDescriptorSetState&) = delete;

        struct DescriptorTableLayout
        {
            D3D12_DESCRIPTOR_HEAP_TYPE _heapType;
            u32 _countDesc;
            bool _direct;
        };

        void composeDescriptorSetTables(const D3DPipelineState* pipeline, std::vector<std::tuple<u32, DescriptorTableLayout, DescriptorTableContainer>>& tables) const;

        void updateDescriptorTable(D3DDescriptorHeap* heap, u32 offset, const DescriptorTableContainer& table, D3D12_DESCRIPTOR_HEAP_TYPE type);

        ID3D12Device* const m_device;
        D3DDescriptorHeapManager& m_heapManager;
        std::map<D3DBinding, D3DBindingResource, D3DBinding::Less> m_boundedDescriptorSets[k_maxDescriptorSetCount];

        std::vector<std::tuple<u32, DescriptorTableLayout, DescriptorTableContainer>> m_updatedTablesCache;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER