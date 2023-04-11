#include "D3DRootSignature.h"
#include "Renderer/Shader.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DDescriptorHeap.h"
#include "D3DDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DRootSignatureCreator::D3DRootSignatureCreator(const ShaderProgramDescription& desc)
    : m_hash(0)
    , m_signature(nullptr)
{
    auto shaderVisibility = [](ShaderType type) -> D3D12_SHADER_VISIBILITY
    {
        switch (type)
        {
        case ShaderType::Vertex:
            return D3D12_SHADER_VISIBILITY_VERTEX;

        case ShaderType::Fragment:
            return D3D12_SHADER_VISIBILITY_PIXEL;

        case ShaderType::Compute:
            return D3D12_SHADER_VISIBILITY_ALL;

        default:
            ASSERT(false, "not supported");
        }

        return D3D12_SHADER_VISIBILITY_ALL;
    };

    if (D3DDeviceCaps::getInstance()->rootSignatureVersion.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
    {
        auto descriptorRangeSort = [](const CD3DX12_DESCRIPTOR_RANGE1& range0, const CD3DX12_DESCRIPTOR_RANGE1& range1) -> bool
        {
            if (range0.RegisterSpace < range1.RegisterSpace)
            {
                if (range0.BaseShaderRegister < range1.BaseShaderRegister)
                {
                    return range0.RangeType < range1.RangeType;
                }
            }

            return false;
        };

        auto descriptorBindingSort = [](const D3DBinding& binding0, const D3DBinding& binding1) -> bool
        {
            if (binding0._space < binding1._space)
            {
                if (binding0._register < binding1._register)
                {
                    return binding0._type < binding1._type;
                }
            }

            return false;
        };

        u32 signatureParameterIndex = 0;
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

        std::vector<CD3DX12_DESCRIPTOR_RANGE1> resourcesDescRanges[toEnumType(ShaderType::Count)][k_maxDescriptorSetCount];
        std::vector<CD3DX12_DESCRIPTOR_RANGE1> samplerDescRanges[toEnumType(ShaderType::Count)][k_maxDescriptorSetCount];

        for (auto shader : desc._shaders)
        {
            if (!shader)
            {
                continue;
            }

            const Shader::ReflectionInfo& info = shader->getReflectionInfo();
            ShaderType shaderType = shader->getShaderType();

            //Resources table
            {
                DescriptorTable table[k_maxDescriptorSetCount];

                for (const auto& image : info._images)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 range = {};
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, image._array, image._binding, image._set, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
                    ASSERT(image._set < D3DDeviceCaps::getInstance()->maxDescriptorSets, "Space index is not supported");

                    resourcesDescRanges[toEnumType(shaderType)][image._set].push_back(range);
                    table[image._set]._bindings.push_back(D3DBinding{image._set, image._binding, image._array, D3D12_DESCRIPTOR_RANGE_TYPE_SRV});
                }

                for (const auto& UAV : info._storageImages)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 range = {};
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAV._array, UAV._binding, UAV._set, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
                    ASSERT(UAV._set < D3DDeviceCaps::getInstance()->maxDescriptorSets, "Space index is not supported");

                    resourcesDescRanges[toEnumType(shaderType)][UAV._set].push_back(range);
                    table[UAV._set]._bindings.push_back(D3DBinding{UAV._set, UAV._binding, UAV._array, D3D12_DESCRIPTOR_RANGE_TYPE_UAV});
                }

                for (const auto& UAV : info._storageBuffers)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 range = {};
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, UAV._binding, UAV._set, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
                    ASSERT(UAV._set < D3DDeviceCaps::getInstance()->maxDescriptorSets, "Space index is not supported");

                    resourcesDescRanges[toEnumType(shaderType)][UAV._set].push_back(range);
                    table[UAV._set]._bindings.push_back(D3DBinding{UAV._set, UAV._binding, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV});

                }

                for (u32 setID = 0; setID < k_maxDescriptorSetCount; ++setID)
                {
                    std::vector<CD3DX12_DESCRIPTOR_RANGE1>& resourcesDescRange = resourcesDescRanges[toEnumType(shaderType)][setID];
                    if (!resourcesDescRange.empty())
                    {
                        std::sort(resourcesDescRange.begin(), resourcesDescRange.end(), descriptorRangeSort);
                        std::sort(table[setID]._bindings.begin(), table[setID]._bindings.end(), descriptorBindingSort);

                        CD3DX12_ROOT_PARAMETER1 param = {};
                        param.InitAsDescriptorTable(static_cast<UINT>(resourcesDescRange.size()), resourcesDescRange.data(), shaderVisibility(shaderType));

                        rootParameters.push_back(param);
                    }

                    if (!table[setID]._bindings.empty())
                    {
                        table[setID]._paramIndex = signatureParameterIndex;
                        for (u32 index = 0; index < table[setID]._bindings.size(); ++index)
                        {
                            const D3DBinding& binding = table[setID]._bindings[index];
                            table[setID]._heapGroups[D3DDescriptorHeap::convertDescriptorTypeToHeapType(binding._type)].push_back(binding);
                        }

                        m_signatureTables.push_back(table[setID]);
                        ++signatureParameterIndex;
                    }
                }
            }

            //Constatbuffer direct tables
            {
                DescriptorTable table;
                table._direct = true; //CBO put to Direct binding

                for (const auto& cbBuffer : info._uniformBuffers)
                {
                    ASSERT(cbBuffer._set < D3DDeviceCaps::getInstance()->maxDescriptorSets, "Space index is not supported");

                    table._bindings.push_back(D3DBinding{ cbBuffer._set, cbBuffer._binding, cbBuffer._array, D3D12_DESCRIPTOR_RANGE_TYPE_CBV });

                    CD3DX12_ROOT_PARAMETER1 param = {};
                    param.InitAsConstantBufferView(cbBuffer._binding, cbBuffer._set, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, shaderVisibility(shaderType));

                    rootParameters.push_back(param);
                }

                if (!table._bindings.empty())
                {
                    table._paramIndex = signatureParameterIndex;
                    for (u32 index = 0; index < table._bindings.size(); ++index)
                    {
                        const D3DBinding& binding = table._bindings[index];
                        table._heapGroups[D3DDescriptorHeap::convertDescriptorTypeToHeapType(binding._type)].push_back(binding);
                    }

                    m_signatureTables.push_back(table);
                    ++signatureParameterIndex;
                }
            }

            //Sampler table
            {
                DescriptorTable table[k_maxDescriptorSetCount];

                for (const auto& sampler : info._samplers)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 range = {};
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, sampler._binding, sampler._set, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
                    ASSERT(sampler._set < D3DDeviceCaps::getInstance()->maxDescriptorSets, "Space index is not supported");

                    samplerDescRanges[toEnumType(shaderType)][sampler._set].push_back(range);
                    table[sampler._set]._bindings.push_back(D3DBinding{ sampler._set, sampler._binding, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER });
                }

                for (u32 setID = 0; setID < k_maxDescriptorSetCount; ++setID)
                {
                    std::vector<CD3DX12_DESCRIPTOR_RANGE1>& samplerDescRange = samplerDescRanges[toEnumType(shaderType)][setID];
                    if (!samplerDescRange.empty())
                    {
                        std::sort(samplerDescRange.begin(), samplerDescRange.end(), descriptorRangeSort);

                        CD3DX12_ROOT_PARAMETER1 param = {};
                        param.InitAsDescriptorTable(static_cast<UINT>(samplerDescRange.size()), samplerDescRange.data(), shaderVisibility(shaderType));

                        rootParameters.push_back(param);
                    }

                    if (!table[setID]._bindings.empty())
                    {
                        table[setID]._paramIndex = signatureParameterIndex;
                        for (u32 index = 0; index < table[setID]._bindings.size(); ++index)
                        {
                            const D3DBinding& binding = table[setID]._bindings[index];
                            table[setID]._heapGroups[D3DDescriptorHeap::convertDescriptorTypeToHeapType(binding._type)].push_back(binding);
                        }

                        m_signatureTables.push_back(table[setID]);
                        ++signatureParameterIndex;
                    }
                }
            }
        }

        D3D12_ROOT_SIGNATURE_FLAGS layoutFlags = 
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        for (auto& param : rootParameters)
        {
            if (param.ShaderVisibility == D3D12_SHADER_VISIBILITY_ALL)
            {
                layoutFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            }
            else if (param.ShaderVisibility == D3D12_SHADER_VISIBILITY_VERTEX)
            {
                layoutFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
            }
            else if (param.ShaderVisibility == D3D12_SHADER_VISIBILITY_PIXEL)
            {
                layoutFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            }
        }

        m_hash = crc32c::Crc32c(reinterpret_cast<u8*>(rootParameters.data()), sizeof(CD3DX12_ROOT_PARAMETER1) * rootParameters.size());

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, layoutFlags);

        ID3DBlob* error;
        HRESULT result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &m_signature, &error);
        if (FAILED(result))
        {
            if (error->GetBufferSize() > 0)
            {
                LOG_ERROR("D3DX12SerializeVersionedRootSignature error %s", error->GetBufferPointer());
                error->Release();
            }

            ASSERT(false, "D3DX12SerializeVersionedRootSignature");
            m_signature->Release();
        }
    }
    else //D3D_ROOT_SIGNATURE_VERSION_1_0
    {
        ASSERT(false, "Signature version is not supported");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DRootSignatureManager::D3DRootSignatureManager(ID3D12Device* device) noexcept
  : m_device(device)
{
    LOG_DEBUG("D3DRootSignatureManager::D3DRootSignatureManager constructor %llx", this);
}

D3DRootSignatureManager::~D3DRootSignatureManager()
{
    LOG_DEBUG("D3DRootSignatureManager::~D3DRootSignatureManager destructor %llx", this);
    ASSERT(m_rootSignatures.empty(), "not empty");
}

std::tuple<ID3D12RootSignature*, SignatureParameters> D3DRootSignatureManager::acquireRootSignature(const ShaderProgramDescription& desc)
{
    D3DRootSignatureCreator signatureCreator(desc);
    auto rooSignature = m_rootSignatures.emplace(signatureCreator.m_hash, nullptr);
    if (rooSignature.second)
    {
        ID3D12RootSignature* rootSignature = nullptr;
        HRESULT result = m_device->CreateRootSignature(0, signatureCreator.m_signature->GetBufferPointer(), signatureCreator.m_signature->GetBufferSize(), DX_IID_PPV_ARGS(&rootSignature));
        if (FAILED(result))
        {
            LOG_ERROR("D3DRootSignatureManager::acquireRootSignature, CreateRootSignature is failed. Error %s", D3DDebug::stringError(result).c_str());
            return std::make_tuple<ID3D12RootSignature*, SignatureParameters>(nullptr, {});
        }

        rooSignature.first->second = rootSignature;
    }

    return std::make_tuple(rooSignature.first->second, signatureCreator.m_signatureTables);
}

void D3DRootSignatureManager::removeAllRootSignatures()
{
    for (auto& singature : m_rootSignatures)
    {
        //Possible DX can caches state and return a same signature for different keys.
        //That is why need to call Release instead SAFE_DELETE
        singature.second->Release();
    }
    m_rootSignatures.clear();
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER