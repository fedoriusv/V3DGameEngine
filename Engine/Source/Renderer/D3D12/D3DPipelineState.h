#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"

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
    * D3DGraphicPipelineState final class. DirectX Render side
    */
    class D3DGraphicPipelineState final : public Pipeline, public D3DResource
    {
    public:

        explicit D3DGraphicPipelineState(ID3D12Device* device, D3DRootSignatureManager* const sigManager) noexcept;
        ~D3DGraphicPipelineState();

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        void destroy() override;

        ID3D12PipelineState* getHandle() const;
        ID3D12RootSignature* getSignatureHandle() const;

        static D3D12_INPUT_CLASSIFICATION convertInputRateToD3DClassification(VertexInputAttribDescription::InputRate rate);
        static D3D12_PRIMITIVE_TOPOLOGY_TYPE convertPrimitiveTopologyTypeToD3DTopology(PrimitiveTopology topology);
        static D3D_PRIMITIVE_TOPOLOGY convertPrimitiveTopologyToD3DTopology(PrimitiveTopology topology, u32 patch);
        static D3D12_FILL_MODE convertPolygonModeToD3DMode(PolygonMode mode);
        static D3D12_CULL_MODE convertCulModeToD3D(CullMode mode);
        static BOOL convertCounterClockwiseToD3D(FrontFace face);
        static D3D12_LOGIC_OP convertLogicOperationToD3D(LogicalOperation op);
        static D3D12_BLEND_OP convertBlenOperationToD3D(BlendOperation op);
        static D3D12_BLEND convertBlendFacorToD3D(BlendFactor factor);
        static D3D12_COMPARISON_FUNC convertDepthFunctionToD3D(CompareOperation op);
        static D3D12_DEPTH_WRITE_MASK convertWriteDepthToD3D(bool enable);

        const std::vector<u32>& getBuffersStrides() const;
        D3D12_PRIMITIVE_TOPOLOGY getTopology() const;

    private:

        bool compileShader(const ShaderHeader* header, const void* source, u32 size) override;

        static bool separateSematic(const std::string& str, std::string& name, u32& id);

        D3D12_SHADER_BYTECODE m_bytecode[ShaderType::ShaderType_Count];

        ID3D12Device* const m_device;
        D3DRootSignatureManager* const m_sigatureManager;

        ID3D12PipelineState* m_pipelineState;
        ID3D12RootSignature* m_rootSignature;

        std::vector<u32> m_buffersStride; //Vertex stride
        D3D12_PRIMITIVE_TOPOLOGY m_topology;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER