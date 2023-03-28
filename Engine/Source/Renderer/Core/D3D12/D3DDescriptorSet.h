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
                    bindingResource._resource._constantBuffer = { buffer, offset, size };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DImage>() && UAV == true)
            {
                auto function = [&bind, &bindingResource](const D3DImage* image, const Image::Subresource& subresource) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    bind._uav = 0;
                    bindingResource._resource._UAVImage = { image, subresource };
                };
                function(resource, args...);
            }
            else if constexpr (std::is_same<TResource, D3DBuffer>() && UAV == true)
            {
                auto function = [&bind, &bindingResource](const D3DBuffer* buffer, u32 offset, u32 size) -> void
                {
                    bind._type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    bind._uav = 1;
                    bindingResource._resource._UAVBuffer = { resource, offset, size };
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

            auto iter = m_boundedDescriptorSets[space].emplace(bind, bindingResource);
            if (!iter.second)
            {
                ASSERT(false, "binding is already presentd inside table");
                return false;
            }

            return true;
        }

        bool updateDescriptorTables(D3DGraphicsCommandList* cmdList, D3DPipelineState* pipeline);
        void updateDescriptorSetStatus(); //Call per submit/present
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

        void updateResourceView(D3DDescriptorHeap* heap, u32 offset, const DescriptorTableContainerRef& table, D3D12_DESCRIPTOR_HEAP_TYPE type);

        ID3D12Device* const m_device;
        D3DDescriptorHeapManager& m_heapManager;
        std::map<D3DBinding, D3DBindingResource, D3DBinding::Less> m_boundedDescriptorSets[k_maxDescriptorSetCount];

        std::vector<ID3D12DescriptorHeap*> m_internalHeaps;
        std::vector<std::tuple<u32, D3DDescriptorHeap*, u32>> m_internalTables;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER