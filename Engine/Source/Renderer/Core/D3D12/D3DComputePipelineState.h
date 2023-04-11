#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DPipelineState.h"

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
    * @brief D3DComputePipelineState final class. DirectX Render side
    */
    class D3DComputePipelineState final : public D3DPipelineState
    {
    public:

        explicit D3DComputePipelineState(ID3D12Device2* device, D3DRootSignatureManager* const signatureManager) noexcept;
        ~D3DComputePipelineState();

    private:

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        bool create(const PipelineComputeInfo* pipelineInfo) override;
        void destroy() override;

        bool compileShader(ShaderType type, const void* source, u32 size) override;

        D3D12_SHADER_BYTECODE m_bytecode;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
