#include "D3DComputePipelineState.h"

#include "Renderer/Shader.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DRootSignature.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DComputePipelineState::D3DComputePipelineState(ID3D12Device2* device, D3DRootSignatureManager* const signatureManager) noexcept
    : D3DPipelineState(PipelineType::PipelineType_Compute, device, signatureManager)
    , m_bytecode()
{
    LOG_DEBUG("D3DComputePipelineState::D3DComputePipelineState constructor %llx", this);
}

D3DComputePipelineState::~D3DComputePipelineState()
{
    LOG_DEBUG("D3DComputePipelineState::~D3DComputePipelineState destructor %llx", this);
}

bool D3DComputePipelineState::create(const PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(false, "must be compute");
    return false;
}

bool D3DComputePipelineState::create(const PipelineComputeInfo* pipelineInfo)
{
    CD3DX12_PIPELINE_STATE_STREAM1 psoDesc = {};
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psoDesc.NodeMask = 0;

    std::tie(m_rootSignature, m_signatureParameters) = m_signatureManager->acquireRootSignature(pipelineInfo->_programDesc);
    ASSERT(m_rootSignature, "nullptr");
    psoDesc.pRootSignature = m_rootSignature;

    //Shader
    {
        const Shader* computeShader = pipelineInfo->_programDesc._shaders[ShaderType::Compute];
        ASSERT(computeShader, "must be present");
        if (!createShader(computeShader))
        {
            LOG_ERROR("D3DComputePipelineState::create is failed to create shader");
            return false;
        }

        ASSERT(m_bytecode.pShaderBytecode && m_bytecode.BytecodeLength > 0, "invalid shader");
        psoDesc.CS = m_bytecode;
    }

    psoDesc.CachedPSO = {};

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    streamDesc.SizeInBytes = sizeof(psoDesc);
    streamDesc.pPipelineStateSubobjectStream = &psoDesc;

    HRESULT result = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(result))
    {
        LOG_ERROR("D3DComputePipelineState::create CreateComputePipelineState is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}

void D3DComputePipelineState::destroy()
{
    SAFE_DELETE(m_pipelineState);
}

bool D3DComputePipelineState::compileShader(const ShaderHeader* header, const void* source, u32 size)
{
    m_bytecode.BytecodeLength = static_cast<u32>(size);
    m_bytecode.pShaderBytecode = source;

    return true;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER