#include "D3DDescriptorSet.h"

#ifdef D3D_RENDER
#include "D3DPipelineState.h"
#include "D3DDescriptorHeap.h"
#include "D3DImage.h"
#include "D3DBuffer.h"
#include "D3DSampler.h"
#include "D3DCommandList.h"

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
    , m_heapManager(manager)
{
}

D3DDescriptorSetState::~D3DDescriptorSetState()
{
    ASSERT(m_usedDescriptorHeaps.empty(), "must be empty");
}

bool D3DDescriptorSetState::updateDescriptorSets(D3DGraphicsCommandList* cmdList, D3DGraphicPipelineState* pipeline)
{
    DescriptorTableContainer table[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    composeDescriptorSetTable(pipeline, table);

    std::vector<ID3D12DescriptorHeap*> heaps;
    std::map<u32, std::tuple<D3DDescriptorHeap*, u32>> tableHeaps;
    heaps.reserve(!table[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].empty() + !table[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].empty());

    for (u32 tableType = 0; tableType < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++tableType)
    {
        if (table[tableType].empty())
        {
            continue;
        }
        
        auto countDescriptorHeaps = [](const DescriptorTableContainer& tables) -> u32
        {
            u32 countHeaps = 0;
            for (auto& bindings : tables)
            {
                countHeaps += static_cast<u32>(bindings.second.size());
            }
            return countHeaps;
        };

        ASSERT(tableType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || tableType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "wrong heap type");
        D3DDescriptorHeap* heap = m_heapManager->allocateDescriptorHeap((D3D12_DESCRIPTOR_HEAP_TYPE)tableType, countDescriptorHeaps(table[tableType]), D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        ASSERT(heap, "nullptr");

        D3DDescriptorSetState::updateDescriptorTable(heap, table[tableType], (D3D12_DESCRIPTOR_HEAP_TYPE)tableType, tableHeaps);

        cmdList->setUsed(heap, 0);
        m_usedDescriptorHeaps.push_back(heap);

        heaps.push_back(heap->getHandle());
    }

    if (!heaps.empty())
    {
        cmdList->setDescriptorTables(heaps, tableHeaps);
        return true;
    }

    return false;
}

void D3DDescriptorSetState::updateStatus()
{
    for (auto descIter = m_usedDescriptorHeaps.begin(); descIter != m_usedDescriptorHeaps.end();)
    {
        D3DDescriptorHeap* heap = (*descIter);
        if (!heap->isUsed())
        {
            m_heapManager->deallocDescriptorHeap(heap);

            descIter = m_usedDescriptorHeaps.erase(descIter);
            continue;
        }
        ++descIter;
    }
}

void D3DDescriptorSetState::invalidateDescriptorSetTable()
{
    m_descriptorSetTable.clear();
}

void D3DDescriptorSetState::composeDescriptorSetTable(const D3DGraphicPipelineState* pipeline, DescriptorTableContainer table[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])
{
    for (auto& binding : m_descriptorSetTable)
    {
        s32 paramIndex = pipeline->getSignatureParameterIndex(binding._space, binding._binding);
        if (paramIndex < 0)
        {
            ASSERT(false, "binding not found");
            continue;
        }

        u32 typeIndex = D3DDescriptorSetState::convertDescriptorTypeToHeapType(binding._type);
        auto iter = table[typeIndex].emplace(paramIndex, std::set<Binding, Binding::Less>{ binding });
        if (!iter.second)
        {
            (*iter.first).second.insert(binding);
        }
    }
}

void D3DDescriptorSetState::updateDescriptorTable(D3DDescriptorHeap* heap, const DescriptorTableContainer& table, D3D12_DESCRIPTOR_HEAP_TYPE type, std::map<u32, std::tuple<D3DDescriptorHeap*, u32>>& descTable)
{
    u32 descriptorOffsetIndex = 0;
    u32 descriptorIndex = 0;
    for (auto& [paramIndex, params] : table)
    {
        ASSERT(!params.empty(), "empty table");
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        {
            for (auto& param : params)
            {
                ASSERT(param._type != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "must be separated");
                switch (param._type)
                {
                case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                {
                    const D3DImage* image = param._resource._image._image;

                    CD3DX12_CPU_DESCRIPTOR_HANDLE imgHandle(heap->getCPUHandle(), descriptorIndex, heap->getIncrement());
                    m_device->CreateShaderResourceView(image->getResource(), &image->getView(), imgHandle);
                    ++descriptorIndex;

                    break;
                }
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                {
                    const D3DBuffer* buffer = param._resource._constantBuffer._buffer;

                    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                    cbvDesc.BufferLocation = buffer->getGPUAddress();
                    cbvDesc.SizeInBytes = core::alignUp<UINT>(param._resource._constantBuffer._size, 256);

                    CD3DX12_CPU_DESCRIPTOR_HANDLE cbHandle(heap->getCPUHandle(), descriptorIndex, heap->getIncrement());
                    m_device->CreateConstantBufferView(&cbvDesc, cbHandle);
                    ++descriptorIndex;

                    break;
                }
                case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                default:
                    ASSERT(false, "not impl");
                }
            }
            break;
        }

        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        {
            for (auto& param : params)
            {
                const D3DSampler* sampler = param._resource._sampler._sampler;

                ASSERT(param._type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "wrond type");
                CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(heap->getCPUHandle(), descriptorIndex, heap->getIncrement());
                m_device->CreateSampler(&sampler->getDesc(), samplerHandle);
                ++descriptorIndex;
            }
            break;
        }

        default:
            ASSERT(false, "can't used here");
        }

        descTable.emplace(paramIndex, std::make_tuple(heap, descriptorOffsetIndex));
        descriptorOffsetIndex += descriptorIndex;
    }
}

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER