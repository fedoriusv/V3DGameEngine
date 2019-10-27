#include "D3D12PipelineState.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3D12Debug.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3DGraphicPipelineState::D3DGraphicPipelineState(ID3D12Device* device) noexcept
    : Pipeline(PipelineType::PipelineType_Graphic)
    , m_device(device)
    , m_pipelineState(nullptr)
{
    LOG_DEBUG("D3DGraphicPipelineState::D3DGraphicPipelineState constructor %llx", this);
}

D3DGraphicPipelineState::~D3DGraphicPipelineState()
{
    LOG_DEBUG("D3DGraphicPipelineState::D3DGraphicPipelineState destructor %llx", this);
    ASSERT(!m_pipelineState, "not nullptr");
}

bool D3DGraphicPipelineState::create(const PipelineGraphicInfo* pipelineInfo)
{
    // Describe and create the graphics pipeline state object (PSO).
    //TODO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    /*psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;*/

    HRESULT result = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(result))
    {
        LOG_ERROR("D3DGraphicPipelineState::create CreateGraphicsPipelineState is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}

void D3DGraphicPipelineState::destroy()
{
    if (m_pipelineState)
    {
        m_pipelineState = nullptr;
    }
}

ID3D12PipelineState* D3DGraphicPipelineState::getHandle() const
{
    ASSERT(m_pipelineState, "nullptr");
    return m_pipelineState;
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
