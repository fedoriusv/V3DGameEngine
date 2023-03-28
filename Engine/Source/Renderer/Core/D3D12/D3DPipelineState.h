#pragma once

#include "Common.h"
#include "Renderer/Core/Pipeline.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"
#include "D3DRootSignature.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DRootSignatureManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DPipelineState class. DirectX Render side
    */
    class D3DPipelineState : public Pipeline, public D3DResource
    {
    public:

        explicit D3DPipelineState(PipelineType type, ID3D12Device2* device, D3DRootSignatureManager* const signatureManager) noexcept;
        ~D3DPipelineState();

        ID3D12PipelineState* getHandle() const;
        ID3D12RootSignature* getSignatureHandle() const;
        const SignatureParameters& getSignatureParameters() const;

    protected:

        ID3D12Device2* const m_device;
        D3DRootSignatureManager* const m_signatureManager;

        ID3D12PipelineState* m_pipelineState;
        ID3D12RootSignature* m_rootSignature;
        SignatureParameters m_signatureParameters;
    };

    inline ID3D12PipelineState* D3DPipelineState::getHandle() const
    {
        ASSERT(m_pipelineState, "nullptr");
        return m_pipelineState;
    }

    inline ID3D12RootSignature* D3DPipelineState::getSignatureHandle() const
    {
        ASSERT(m_rootSignature, "nullptr");
        return m_rootSignature;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
