#include "D3DDescriptorSet.h"
#include "Utils/Logger.h"

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

D3D12_DESCRIPTOR_HEAP_TYPE D3DDescriptorSetState::convertDescriptorTypeToHeapType(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType)
{
    switch (descriptorType)
    {
    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    default:
        ASSERT(false, "unsupported");
    }

    return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
}

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

bool D3DDescriptorSetState::updateDescriptorSets(D3DGraphicsCommandList* cmdList, D3DPipelineState* pipeline)
{
    m_updatedTablesCache.clear();
    composeDescriptorSetTables(pipeline, m_updatedTablesCache);


    std::vector<ID3D12DescriptorHeap*> heaps;
    std::vector<std::tuple<u32, D3DDescriptorHeap*, u32>> tables;
    for (u32 paramIndex = 0; paramIndex < m_updatedTablesCache.size(); ++paramIndex)
    {
        const auto& tableData = m_updatedTablesCache[paramIndex];
        if (std::get<2>(tableData).empty())
        {
            continue;
        }

        if (std::get<1>(tableData)._direct) // direct binding
        {
            for (auto& [binding, resource] : std::get<2>(tableData))
            {
                switch (binding._type)
                {
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                {
                    const D3DBuffer* buffer = resource._resource._constantBuffer._buffer;
                    u32 offset = resource._resource._constantBuffer._offset;
                    cmdList->setConstantBuffer(paramIndex, buffer, offset, pipeline->getType());

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
            auto [heap, offset] = m_heapManager.getDescriptor(D3DDescriptorHeapManager::DescriptorInfo(std::get<2>(tableData), std::get<0>(tableData)));
            if (!heap)
            {
               std::tie(heap, offset) = m_heapManager.acquireDescriptorHeap(std::get<1>(tableData)._heapType, std::get<1>(tableData)._countDesc);
               ASSERT(heap, "nullptr");

               [[maybe_unused]] bool success = m_heapManager.addDescriptor(D3DDescriptorHeapManager::DescriptorInfo(std::get<2>(tableData), std::get<0>(tableData)), {heap, offset});
               ASSERT(success, "already is presented");

               D3DDescriptorSetState::updateDescriptorTable(heap, offset, std::get<2>(tableData), std::get<1>(tableData)._heapType);
            }
            cmdList->setUsed(heap, 0);
            for (auto& [binding, resource] : std::get<2>(tableData))
            {
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
                    if (const D3DImage* UAVImage = resource._resource._UAV._image._image; UAVImage)
                    {
                        cmdList->setUsed(const_cast<D3DImage*>(UAVImage), 0);
                    }

                    if (const D3DBuffer* UAVBuffer = resource._resource._UAV._buffer._buffer; UAVBuffer)
                    {
                        cmdList->setUsed(const_cast<D3DBuffer*>(UAVBuffer), 0);
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

            heaps.push_back(heap->getHandle());
            tables.emplace_back(paramIndex, heap, offset);
        }
    }

    if (!heaps.empty())
    {
        cmdList->setDescriptorTables(heaps, tables, pipeline->getType());
    }

    return false;
}

void D3DDescriptorSetState::updateStatus()
{
    m_heapManager.updateDescriptorHeaps();
}

void D3DDescriptorSetState::invalidateDescriptorSetTable()
{
    for (u32 setIndex = 0; setIndex < k_maxDescriptorSetCount; ++setIndex)
    {
        m_descriptorSets[setIndex].clear();
    }
}

void D3DDescriptorSetState::composeDescriptorSetTables(const D3DPipelineState* pipeline, std::vector<std::tuple<u32, DescriptorTableLayout, DescriptorTableContainer>>& tables) const
{
    tables.clear();

    for (u32 setIndex = 0; setIndex < k_maxDescriptorSetCount; ++setIndex)
    {
        u32 hash = 0;
        for (const auto& binding : m_descriptorSets[setIndex])
        {
            s32 paramIndex = pipeline->getSignatureParameterIndex(binding.first);
            if (paramIndex < 0)
            {
                ASSERT(false, "binding not found");
                continue;
            }

            if (paramIndex >= tables.size())
            {
                tables.resize(m_descriptorSets[setIndex].size());
            }
            ASSERT(paramIndex < tables.size(), "range out");

            hash = crc32c::Extend(hash, reinterpret_cast<const u8*>(&binding.second), sizeof(D3DBindingResource));

            std::get<0>(tables[paramIndex]) = hash;
            std::get<1>(tables[paramIndex]) = { D3DDescriptorSetState::convertDescriptorTypeToHeapType(binding.first._type), (u32)m_descriptorSets[setIndex].size(), (bool)binding.first._direct};
            std::get<2>(tables[paramIndex]).emplace_back(binding.first, binding.second);
        }
    }
}

void D3DDescriptorSetState::updateDescriptorTable(D3DDescriptorHeap* heap, u32 offset, const DescriptorTableContainer& table, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    u32 descriptorOffsetIndex = offset;
    u32 descriptorIndex = 0;
    for (auto& [binding, resorce] : table)
    {
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        {
            ASSERT(binding._type != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "must be separated");
            switch (binding._type)
            {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            {
                const D3DImage* image = resorce._resource._image._image;
                const Image::Subresource& subresource = resorce._resource._image._subresource;

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
                const D3DBuffer* buffer = resorce._resource._constantBuffer._buffer;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.BufferLocation = buffer->getGPUAddress();
                cbvDesc.SizeInBytes = core::alignUp<UINT>(resorce._resource._constantBuffer._size, 256);

                CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(heap->getCPUHandle(), offset + descriptorIndex, heap->getIncrement());
                m_device->CreateConstantBufferView(&cbvDesc, cbHandle);
                ++descriptorIndex;

                break;
            }
            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            {
                ID3D12Resource* uavResource = nullptr;
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

                const D3DImage* UAVImage = resorce._resource._UAV._image._image;
                const D3DBuffer* UAVBuffer = resorce._resource._UAV._buffer._buffer;
                if (UAVImage)
                {
                    uavResource = UAVImage->getResource();

                    const Image::Subresource& subresource = resorce._resource._UAV._image._subresource;
                    uavDesc = UAVImage->getView<D3D12_UNORDERED_ACCESS_VIEW_DESC>(subresource);
                }
                else if (UAVBuffer)
                {
                    ASSERT(false, "not impl");
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
            const D3DSampler* sampler = resorce._resource._sampler._sampler;

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