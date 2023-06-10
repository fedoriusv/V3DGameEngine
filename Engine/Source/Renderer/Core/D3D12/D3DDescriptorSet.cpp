#include "D3DDescriptorSet.h"
#include "Utils/Logger.h"
#include "Renderer/Core/RenderFrameProfiler.h"

#ifdef D3D_RENDER
#include "D3DPipelineState.h"
#include "D3DDescriptorHeap.h"
#include "D3DImage.h"
#include "D3DBuffer.h"
#include "D3DSampler.h"
#include "D3DCommandList.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

#if FRAME_PROFILER_ENABLE
extern RenderFrameProfiler* g_d3dCPUProfiler;
#endif //FRAME_PROFILER_ENABLE

D3DDescriptorSetState::D3DDescriptorSetState(ID3D12Device* device, D3DDescriptorHeapManager* manager) noexcept
    : m_device(device)
    , m_heapManager(*manager)
{
    LOG_DEBUG("D3DDescriptorSetState::D3DDescriptorSetState constructor %llx", this);
}

D3DDescriptorSetState::~D3DDescriptorSetState()
{
    LOG_DEBUG("D3DDescriptorSetState::~D3DDescriptorSetState destructor %llx", this);
}

bool D3DDescriptorSetState::updateDescriptorTables(D3DGraphicsCommandList* cmdList, D3DPipelineState* pipeline)
{
    m_internalHeaps.clear();
    m_internalTables.clear();

    for (auto& descriptorTable : pipeline->getSignatureParameters())
    {
        if (descriptorTable._direct)
        {
            for (auto& binding : descriptorTable._bindings)
            {
                if (m_boundedDescriptorSets[binding._space].empty())
                {
                    continue;
                }

                auto found = m_boundedDescriptorSets[binding._space].find(binding);
                ASSERT(found != m_boundedDescriptorSets[binding._space].end(), "not found");
                const D3DBindingResource& resource = found->second;

                switch (binding._type)
                {
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                {
                    const D3DBuffer* buffer = resource._resource._constantBuffer._buffer;
                    u32 offset = resource._resource._constantBuffer._offset;
                    cmdList->setDirectConstantBuffer(descriptorTable._paramIndex, buffer, offset, pipeline->getType());

                    cmdList->setUsed(const_cast<D3DBuffer*>(buffer), 0);
                    break;
                }

                default:
                    ASSERT(false, "supported constant buffer only");
                }
            }
        }
        else
        {
            for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
                u32 hash = 0;
                u32 resourceCount = 0;
                std::array<D3DBindingResource, k_maxDescriptorBindingCount> resources;

                D3D12_DESCRIPTOR_HEAP_TYPE heapType = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
                for (auto& binding : descriptorTable._heapGroups[heapType])
                {
                    if (m_boundedDescriptorSets[binding._space].empty())
                    {
                        continue;
                    }

                    auto found = m_boundedDescriptorSets[binding._space].find(binding);
                    ASSERT(found != m_boundedDescriptorSets[binding._space].end(), "not found");
                    const D3DBindingResource& resource = found->second;
                    resources[resourceCount++] = resource;

                    hash = crc32c::Extend(hash, reinterpret_cast<const u8*>(&resource), sizeof(D3DBindingResource));
                    switch (binding._type)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                    {
                        const D3DImage* image = resource._resource._image._image;
                        cmdList->setUsed(const_cast<D3DImage*>(image), 0);
                        break;
                    }

                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                    {
                        if (binding._uav)
                        {
                            const D3DBuffer* UAVBuffer = resource._resource._UAVBuffer._buffer;
                            cmdList->setUsed(const_cast<D3DBuffer*>(UAVBuffer), 0);
                        }
                        else
                        {
                            const D3DImage* UAVImage = resource._resource._UAVImage._image;
                            cmdList->setUsed(const_cast<D3DImage*>(UAVImage), 0);
                        }
                        break;
                    }

                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                    {
                        const D3DBuffer* buffer = resource._resource._constantBuffer._buffer;
                        cmdList->setUsed(const_cast<D3DBuffer*>(buffer), 0);
                        break;
                    }

                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    {
                        const D3DSampler* sampler = resource._resource._sampler._sampler;
                        cmdList->setUsed(const_cast<D3DSampler*>(sampler), 0);
                        break;
                    }

                    default:
                        ASSERT(false, "not impl");
                    }
                }

                if (resourceCount > 0)
                {
                    D3DDescriptorHeapManager::DescriptorTableInfo descriptor(descriptorTable._heapGroups[heapType], resources, hash); //TODO expensive copy. rework
                    auto [heap, offset] = m_heapManager.getDescriptor(descriptor);
                    if (!heap)
                    {
                        std::tie(heap, offset) = m_heapManager.acquireDescriptorHeap(heapType, descriptorTable._heapGroups[heapType].size());
                        ASSERT(heap, "nullptr");

                        [[maybe_unused]] bool success = m_heapManager.addDescriptor(descriptor, { heap, offset });
                        ASSERT(success, "already is presented");

                        D3DDescriptorSetState::updateResourceView(heap, offset, { descriptorTable._heapGroups[heapType], resources }, heapType);
                    }
                    cmdList->setUsed(heap, 0);

                    m_internalHeaps.push_back(heap->getHandle());
                    m_internalTables.emplace_back(descriptorTable._paramIndex, heap, offset);
                }
            }
        }
    }

    if (!m_internalTables.empty())
    {
        cmdList->setDescriptorTables(m_internalHeaps, m_internalTables, pipeline->getType());
    }

    return true;
}

void D3DDescriptorSetState::updateDescriptorSetStatus()
{
    m_heapManager.updateDescriptorHeaps();
}

void D3DDescriptorSetState::invalidateDescriptorSetTable()
{
    for (u32 setIndex = 0; setIndex < k_maxDescriptorSetCount; ++setIndex)
    {
        m_boundedDescriptorSets[setIndex].clear();
    }
}

void D3DDescriptorSetState::updateResourceView(D3DDescriptorHeap* heap, u32 offset, const DescriptorTableContainerRef& table, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    u32 descriptorOffsetIndex = offset;
    u32 descriptorIndex = 0;

    const std::vector<D3DBinding>& bindings = std::get<0>(table);
    const std::array<D3DBindingResource, k_maxDescriptorBindingCount>& resources = std::get<1>(table);
    for (u32 index = 0; index < bindings.size(); ++index)
    {
        const D3DBinding& binding = bindings[index];
        const D3DBindingResource& resource = resources[index];
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        {
            ASSERT(binding._type != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "must be separated");
            switch (binding._type)
            {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            {
                const D3DImage* image = resource._resource._image._image;
                const Image::Subresource& subresource = resource._resource._image._subresource;

                CD3DX12_CPU_DESCRIPTOR_HANDLE imgHandle(heap->getCPUHandle(), offset + descriptorIndex, heap->getIncrement());
                if (const D3DImage* resolveImage = image->getResolveImage())
                {
                    m_device->CreateShaderResourceView(resolveImage->getResource(), &resolveImage->getView<D3D12_SHADER_RESOURCE_VIEW_DESC>(subresource), imgHandle);
                }
                else
                {
                    m_device->CreateShaderResourceView(image->getResource(), &image->getView<D3D12_SHADER_RESOURCE_VIEW_DESC>(subresource), imgHandle);
                }
                ++descriptorIndex;

                break;
            }
            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            {
                const D3DBuffer* buffer = resource._resource._constantBuffer._buffer;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.BufferLocation = buffer->getGPUAddress();
                cbvDesc.SizeInBytes = math::alignUp<UINT>(resource._resource._constantBuffer._size, 256);

                CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(heap->getCPUHandle(), offset + descriptorIndex, heap->getIncrement());
                m_device->CreateConstantBufferView(&cbvDesc, cbHandle);
                ++descriptorIndex;

                break;
            }
            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            {
                ID3D12Resource* uavResource = nullptr;
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

                if (binding._uav)
                {
                    const D3DBuffer* UAVBuffer = resource._resource._UAVBuffer._buffer;
                    ASSERT(false, "get View impl");
                }
                else
                {
                    const D3DImage* UAVImage = resource._resource._UAVImage._image;
                    uavResource = UAVImage->getResource();

                    const Image::Subresource& subresource = resource._resource._UAVImage._subresource;
                    uavDesc = UAVImage->getView<D3D12_UNORDERED_ACCESS_VIEW_DESC>(subresource);
                }
                ASSERT(uavResource, "nullptr");

                CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(heap->getCPUHandle(), offset + descriptorIndex, heap->getIncrement());
                m_device->CreateUnorderedAccessView(uavResource, nullptr, &uavDesc, uavHandle);
                ++descriptorIndex;

                break;
            }

            default:
                ASSERT(false, "not impl");
            }
            break;
        }

        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        {
            const D3DSampler* sampler = resource._resource._sampler._sampler;

            ASSERT(binding._type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "wrond type");
            CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(heap->getCPUHandle(), offset + descriptorIndex, heap->getIncrement());
            m_device->CreateSampler(&sampler->getDesc(), samplerHandle);
            ++descriptorIndex;

            break;
        }

        default:
            ASSERT(false, "can't used here");
        }

        descriptorOffsetIndex += descriptorIndex;
    }
}

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER