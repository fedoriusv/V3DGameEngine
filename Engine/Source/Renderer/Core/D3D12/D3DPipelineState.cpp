#include "D3DPipelineState.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DPipelineState::D3DPipelineState(PipelineType type, ID3D12Device2* device, D3DRootSignatureManager* const signatureManager) noexcept
    : Pipeline(type)
    , m_device(device)
    , m_signatureManager(signatureManager)

    , m_pipelineState(nullptr)
    , m_rootSignature(nullptr)
{
}

D3DPipelineState::~D3DPipelineState()
{
    ASSERT(!m_pipelineState, "not nullptr");
}

ID3D12PipelineState* D3DPipelineState::getHandle() const
{
    ASSERT(m_pipelineState, "nullptr");
    return m_pipelineState;
}

ID3D12RootSignature* D3DPipelineState::getSignatureHandle() const
{
    ASSERT(m_rootSignature, "nullptr");
    return m_rootSignature;
}

u32 D3DPipelineState::getSignatureParameterIndex(const D3DBinding& binding) const
{
    auto iter = m_signatureParameters.find(binding);
    if (iter != m_signatureParameters.end())
    {
        return iter->second;
    }

    return -1;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER