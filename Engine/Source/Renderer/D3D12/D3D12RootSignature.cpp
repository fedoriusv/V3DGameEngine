#include "D3D12RootSignature.h"
#include "Renderer/Shader.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

#ifdef D3D_RENDER
#include "D3D12DeviceCaps.h"
#include "D3D12Debug.h"
#include "d3dx12.h"

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
        case ShaderType::ShaderType_Vertex:
            return D3D12_SHADER_VISIBILITY_VERTEX;

        case ShaderType::ShaderType_Fragment:
            return D3D12_SHADER_VISIBILITY_PIXEL;
        }

        ASSERT(false, "not supported");
        return D3D12_SHADER_VISIBILITY_ALL;
    };

    if (D3DDeviceCaps::getInstance()->rootSignatureVersion.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
    {
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
        std::vector<CD3DX12_DESCRIPTOR_RANGE1> descRanges[ShaderType::ShaderType_Count];
        for (auto shader : desc._shaders)
        {
            const Shader::ReflectionInfo& info = shader->getReflectionInfo();
            ShaderType shaderType = shader->getShaderHeader()._type;
            for (auto cbBuffer : info._uniformBuffers)
            {
                CD3DX12_DESCRIPTOR_RANGE1 range = {};
                range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbBuffer._array, cbBuffer._binding, cbBuffer._set, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

                descRanges[shaderType].push_back(range);
            }

            for (auto image : info._images)
            {
                CD3DX12_DESCRIPTOR_RANGE1 range = {};
                range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, image._array, image._binding, image._set, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

                descRanges[shaderType].push_back(range);
            }

            for (auto image : info._samplers)
            {
                CD3DX12_DESCRIPTOR_RANGE1 range = {};
                range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, image._binding, image._set, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

                descRanges[shaderType].push_back(range);
            }

            ASSERT(info._sampledImages.empty(), "not supported");

            if (!descRanges[shaderType].empty())
            {
                CD3DX12_ROOT_PARAMETER1 param = {};
                param.InitAsDescriptorTable(static_cast<UINT>(descRanges[shaderType].size()), descRanges[shaderType].data(), shaderVisibility(shaderType));

                rootParameters.push_back(param);
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
            else if (param.ShaderVisibility == D3D12_SHADER_VISIBILITY_VERTEX)
            {
                layoutFlags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            }
        }

        m_hash = crc32c::Crc32c(reinterpret_cast<u8*>(rootParameters.data()), sizeof(CD3DX12_ROOT_PARAMETER1) * rootParameters.size());

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, layoutFlags);

        ID3DBlob* error;
        HRESULT result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3DDeviceCaps::getInstance()->rootSignatureVersion.HighestVersion, &m_signature, &error);
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
    else
    {
        ASSERT(false, "TODO");
        //rootSignatureDesc.Init_1_0(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

D3DRootSignatureManager::D3DRootSignatureManager(ID3D12Device* device) noexcept
  : m_device(device)
{
}

D3DRootSignatureManager::~D3DRootSignatureManager()
{
    //remove all sig
}

ID3D12RootSignature* D3DRootSignatureManager::acquireRootSignature(const ShaderProgramDescription& desc)
{
    D3DRootSignatureCreator signatureCreator(desc);
    auto rooSignature = m_rootSignatures.emplace(signatureCreator.m_hash, nullptr);
    if (!rooSignature.second)
    {
        return rooSignature.first->second;
    }
    else
    {
        ID3D12RootSignature* rootSignature = rooSignature.first->second;
        HRESULT result = m_device->CreateRootSignature(0, signatureCreator.m_signature->GetBufferPointer(), signatureCreator.m_signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(result))
        {
            LOG_ERROR("D3DRootSignatureManager::acquireRootSignature, CreateRootSignature is failed. Error %s", D3DDebug::stringError(result));
            return nullptr;
        }

        return rootSignature;
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER