#include "D3DGraphicPipelineState.h"

#include "Renderer/Shader.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DImage.h"
#include "D3DRootSignature.h"
#include "D3DDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3D12_INPUT_CLASSIFICATION D3DGraphicPipelineState::convertInputRateToD3DClassification(VertexInputAttribDescription::InputRate rate)
{
    switch (rate)
    {
    case VertexInputAttribDescription::InputRate::InputRate_Vertex:
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

    case VertexInputAttribDescription::InputRate::InputRate_Instance:
        return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    }

    return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE D3DGraphicPipelineState::convertPrimitiveTopologyTypeToD3DTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::PrimitiveTopology_PointList:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    case PrimitiveTopology::PrimitiveTopology_LineList:
    case PrimitiveTopology::PrimitiveTopology_LineStrip:
    case PrimitiveTopology::PrimitiveTopology_LineListWithAdjacency:
    case PrimitiveTopology::PrimitiveTopology_LineStripWithAdjacency:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    case PrimitiveTopology::PrimitiveTopology_TriangleList:
    case PrimitiveTopology::PrimitiveTopology_TriangleStrip:
    case PrimitiveTopology::PrimitiveTopology_TriangleFan:
    case PrimitiveTopology::PrimitiveTopology_TriangleListWithAdjacency:
    case PrimitiveTopology::PrimitiveTopology_TriangleStripWithAdjacency:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    case PrimitiveTopology::PrimitiveTopology_PatchList:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    ASSERT(false, "nuknown");
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

D3D_PRIMITIVE_TOPOLOGY D3DGraphicPipelineState::convertPrimitiveTopologyToD3DTopology(PrimitiveTopology topology, u32 patch)
{
    switch (topology)
    {
    case PrimitiveTopology::PrimitiveTopology_PointList:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

    case PrimitiveTopology::PrimitiveTopology_LineList:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

    case PrimitiveTopology::PrimitiveTopology_LineStrip:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

    case PrimitiveTopology::PrimitiveTopology_TriangleList:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    case PrimitiveTopology::PrimitiveTopology_TriangleStrip:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    case PrimitiveTopology::PrimitiveTopology_TriangleFan:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    case PrimitiveTopology::PrimitiveTopology_LineListWithAdjacency:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

    case PrimitiveTopology::PrimitiveTopology_LineStripWithAdjacency:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

    case PrimitiveTopology::PrimitiveTopology_TriangleListWithAdjacency:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

    case PrimitiveTopology::PrimitiveTopology_TriangleStripWithAdjacency:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

    case PrimitiveTopology::PrimitiveTopology_PatchList:
        return (D3D_PRIMITIVE_TOPOLOGY)((u32)D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + patch);

    default:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    ASSERT(false, "not found");
    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

D3D12_FILL_MODE D3DGraphicPipelineState::convertPolygonModeToD3DMode(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::PolygonMode_Fill:
        return D3D12_FILL_MODE_SOLID;

    case PolygonMode::PolygonMode_Line:
        return D3D12_FILL_MODE_WIREFRAME;

    case PolygonMode::PolygonMode_Point:
    default:
        ASSERT(false, "not suppotred");
    }

    return D3D12_FILL_MODE_SOLID;
}

D3D12_CULL_MODE D3DGraphicPipelineState::convertCullModeToD3D(CullMode mode)
{
    switch (mode)
    {
    case CullMode::CullMode_None:
        return D3D12_CULL_MODE_NONE;

    case CullMode::CullMode_Back:
        return D3D12_CULL_MODE_BACK;

    case CullMode::CullMode_Front:
        return D3D12_CULL_MODE_FRONT;

    case CullMode::CullMode_FrontAndBack:
    default:

        ASSERT(false, "not suppotrted");
    }

    return D3D12_CULL_MODE_NONE;
}

BOOL D3DGraphicPipelineState::convertCounterClockwiseToD3D(FrontFace face)
{
    return (face == FrontFace::FrontFace_CounterClockwise) ? true : false;
}

D3D12_LOGIC_OP D3DGraphicPipelineState::convertLogicOperationToD3D(LogicalOperation op)
{
    switch (op)
    {
    case LogicalOperation::LogicalOp_Clear:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_CLEAR;

    case LogicalOperation::LogicalOp_And:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND;

    case LogicalOperation::LogicalOp_AndReverse:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND_REVERSE;

    case LogicalOperation::LogicalOp_Copy:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY;

    case LogicalOperation::LogicalOp_AndInverted:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND_INVERTED;

    case LogicalOperation::LogicalOp_NoOp:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOOP;

    case LogicalOperation::LogicalOp_Xor:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_XOR;

    case LogicalOperation::LogicalOp_Or:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR;

    case LogicalOperation::LogicalOp_Nor:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOR;

    case LogicalOperation::LogicalOp_Equivalent:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_EQUIV;

    case LogicalOperation::LogicalOp_Invert:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_INVERT;

    case LogicalOperation::LogicalOp_OrReverse:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR_INVERTED;

    case LogicalOperation::LogicalOp_CopyInverted:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY_INVERTED;

    case LogicalOperation::LogicalOp_OrInverted:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR_INVERTED;

    case LogicalOperation::LogicalOp_Nand:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NAND;

    case LogicalOperation::LogicalOp_Set:
        return D3D12_LOGIC_OP::D3D12_LOGIC_OP_SET;
    }

    ASSERT(false, "not found");
    return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND;
}

D3D12_BLEND_OP D3DGraphicPipelineState::convertBlenOperationToD3D(BlendOperation op)
{
    switch (op)
    {
    case BlendOperation::BlendOp_Add:
        return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

    case BlendOperation::BlendOp_Subtract:
        return D3D12_BLEND_OP::D3D12_BLEND_OP_SUBTRACT;

    case BlendOperation::BlendOp_ReverseSubtract:
        return D3D12_BLEND_OP::D3D12_BLEND_OP_REV_SUBTRACT;

    case BlendOperation::BlendOp_Min:
        return D3D12_BLEND_OP::D3D12_BLEND_OP_MIN;

    case BlendOperation::BlendOp_Max:
        return D3D12_BLEND_OP::D3D12_BLEND_OP_MAX;
    }

    ASSERT(false, "not found");
    return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
}

D3D12_BLEND D3DGraphicPipelineState::convertBlendFacorToD3D(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::BlendFactor_Zero:
        return D3D12_BLEND::D3D12_BLEND_ZERO;

    case BlendFactor::BlendFactor_One:
        return D3D12_BLEND::D3D12_BLEND_ONE;

    case BlendFactor::BlendFactor_SrcColor:
        return D3D12_BLEND::D3D12_BLEND_SRC_COLOR;

    case BlendFactor::BlendFactor_OneMinusSrcColor:
        return D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;

    case BlendFactor::BlendFactor_DstColor:
        return D3D12_BLEND::D3D12_BLEND_DEST_COLOR;

    case BlendFactor::BlendFactor_OneMinusDstColor:
        return D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR;

    case BlendFactor::BlendFactor_SrcAlpha:
        return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;

    case BlendFactor::BlendFactor_OneMinusSrcAlpha:
        return D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;

    case BlendFactor::BlendFactor_DstAlpha:
        return D3D12_BLEND::D3D12_BLEND_DEST_ALPHA;

    case BlendFactor::BlendFactor_OneMinusDstAlpha:
        return D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA;

    case BlendFactor::BlendFactor_SrcAlphaSaturate:
        return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA_SAT;

    case BlendFactor::BlendFactor_Src1Color:
        return D3D12_BLEND::D3D12_BLEND_SRC1_COLOR;

    case BlendFactor::BlendFactor_OneMinusSrc1Color:
        return D3D12_BLEND::D3D12_BLEND_INV_SRC1_COLOR;

    case BlendFactor::BlendFactor_Src1Alpha:
        return D3D12_BLEND::D3D12_BLEND_SRC1_ALPHA;

    case BlendFactor::BlendFactor_OneMinusSrc1Alpha:
        return D3D12_BLEND::D3D12_BLEND_INV_SRC1_ALPHA;

    case BlendFactor::BlendFactor_ConstantColor:
    case BlendFactor::BlendFactor_OneMinusConstantColor:
    case BlendFactor::BlendFactor_ConstantAlpha:
    case BlendFactor::BlendFactor_OneMinusConstantAlpha:
    default:
        ASSERT(false, "not supported");
    }

    ASSERT(false, "not found");
    return D3D12_BLEND::D3D12_BLEND_ZERO;
}

D3D12_COMPARISON_FUNC D3DGraphicPipelineState::convertDepthFunctionToD3D(CompareOperation op)
{
    switch (op)
    {
    case CompareOperation::CompareOp_Never:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;

    case CompareOperation::CompareOp_Less:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;

    case CompareOperation::CompareOp_Equal:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;

    case CompareOperation::CompareOp_LessOrEqual:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;

    case CompareOperation::CompareOp_Greater:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;

    case CompareOperation::CompareOp_NotEqual:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;

    case CompareOperation::CompareOp_GreaterOrEqual:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;

    case CompareOperation::CompareOp_Always:
        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;

    }

    ASSERT(false, "not found");
    return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
}

D3D12_DEPTH_WRITE_MASK D3DGraphicPipelineState::convertWriteDepthToD3D(bool enable)
{
    return (enable) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
}

D3DGraphicPipelineState::D3DGraphicPipelineState(ID3D12Device2* device, D3DRootSignatureManager* const signatureManager) noexcept
    : D3DPipelineState(PipelineType::PipelineType_Graphic, device, signatureManager)
    , m_topology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{
    LOG_DEBUG("D3DGraphicPipelineState::D3DGraphicPipelineState constructor %llx", this);

    memset(m_bytecode, 0, sizeof(m_bytecode[toEnumType(ShaderType::Count)]));
}

D3DGraphicPipelineState::~D3DGraphicPipelineState()
{
    LOG_DEBUG("D3DGraphicPipelineState::~D3DGraphicPipelineState destructor %llx", this);
}

bool D3DGraphicPipelineState::create(const PipelineGraphicInfo* pipelineInfo)
{
#if defined(PLATFORM_XBOX)
    using CD3DX12_PIPELINE_STATE_STREAM1 = CD3DX12_PIPELINE_STATE_STREAM;
#endif
    CD3DX12_PIPELINE_STATE_STREAM1 psoDesc = {};

    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psoDesc.NodeMask = 0;

    std::tie(m_rootSignature, m_signatureParameters) = m_signatureManager->acquireRootSignature(pipelineInfo->_programDesc);
    ASSERT(m_rootSignature, "nullptr");
    psoDesc.pRootSignature = m_rootSignature;

    //Shaders
    {
        memset(m_bytecode, 0, sizeof(m_bytecode));
        for (auto& shader : pipelineInfo->_programDesc._shaders)
        {
            if (!shader)
            {
                continue;
            }

            if (!createShader(shader))
            {
                LOG_ERROR("D3DGraphicPipelineState::create is failed to create shader");
                memset(m_bytecode, 0, sizeof(m_bytecode));

                return false;
            }
        }

        for (u32 type = toEnumType(ShaderType::Vertex); type < (u32)toEnumType(ShaderType::Count); ++type)
        {
            if (m_bytecode[type].BytecodeLength == 0)
            {
                continue;
            }

            switch ((ShaderType)type)
            {
            case ShaderType::Vertex:
                psoDesc.VS = m_bytecode[toEnumType(ShaderType::Vertex)];
                break;

            case ShaderType::Fragment:
                psoDesc.PS = m_bytecode[toEnumType(ShaderType::Fragment)];
                break;
            }
        }
    }

    //StreamOutput
    psoDesc.StreamOutput = {};
    psoDesc.IBStripCutValue = {};

    //Input State
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementsDesc;
    inputElementsDesc.resize(pipelineInfo->_pipelineDesc._vertexInputState._inputAttributes._countInputAttributes);
    std::vector<std::string> semanticNames;
    semanticNames.resize(pipelineInfo->_pipelineDesc._vertexInputState._inputAttributes._countInputAttributes);
    {
        auto& inputState = pipelineInfo->_pipelineDesc._vertexInputState;
        for (u32 i = 0; i < inputElementsDesc.size(); ++i)
        {
            auto& inputBinding = inputState._inputAttributes._inputBindings[inputState._inputAttributes._inputAttributes[i]._bindingId];
            const Shader::Attribute& attribute = pipelineInfo->_programDesc._shaders[toEnumType(ShaderType::Vertex)]->getReflectionInfo()._inputAttribute[i];

            u32 semanticIndex = 0;
#if USE_STRING_ID_SHADER
            separateSematic(attribute._name, semanticNames[i], semanticIndex);
#else
            ASSERT(false, "not impl");
#endif
            D3D12_INPUT_ELEMENT_DESC& elementDesc = inputElementsDesc[i];
            elementDesc.SemanticName = semanticNames[i].data();
            elementDesc.SemanticIndex = semanticIndex;
            elementDesc.Format = D3DImage::convertImageFormatToD3DFormat(inputState._inputAttributes._inputAttributes[i]._format);
            elementDesc.InputSlot = inputState._inputAttributes._inputAttributes[i]._streamId;
            elementDesc.AlignedByteOffset = inputState._inputAttributes._inputAttributes[i]._offest;

            elementDesc.InputSlotClass = D3DGraphicPipelineState::convertInputRateToD3DClassification(inputBinding._rate);
            elementDesc.InstanceDataStepRate = inputBinding._rate == VertexInputAttribDescription::InputRate_Instance ? inputBinding._stride : 0;

            m_buffersStride.push_back(inputBinding._stride);
        }

        D3D12_INPUT_LAYOUT_DESC inputLayout = {};
        inputLayout.NumElements = static_cast<UINT>(inputElementsDesc.size());
        inputLayout.pInputElementDescs = inputElementsDesc.data();

        psoDesc.InputLayout = inputLayout;

        psoDesc.PrimitiveTopologyType = D3DGraphicPipelineState::convertPrimitiveTopologyTypeToD3DTopology(inputState._primitiveTopology);
        m_topology = D3DGraphicPipelineState::convertPrimitiveTopologyToD3DTopology(inputState._primitiveTopology, 0);
    }

    //Rasterizer State
    {
        auto& rasterState = pipelineInfo->_pipelineDesc._rasterizationState;

        D3D12_RASTERIZER_DESC& state = psoDesc.RasterizerState;
        state.FillMode = D3DGraphicPipelineState::convertPolygonModeToD3DMode(rasterState._polygonMode);
        state.CullMode = D3DGraphicPipelineState::convertCullModeToD3D(rasterState._cullMode);
        state.FrontCounterClockwise = D3DGraphicPipelineState::convertCounterClockwiseToD3D(rasterState._frontFace);
        if (rasterState._depthBiasConstant != 0.f || rasterState._depthBiasClamp != 0.f || rasterState._depthBiasSlope != 0.f)
        {
            state.DepthBias = static_cast<INT>(std::roundf(rasterState._depthBiasConstant));
            state.DepthBiasClamp = rasterState._depthBiasClamp;
            state.SlopeScaledDepthBias = rasterState._depthBiasSlope;
        }
        else
        {
            state.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            state.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            state.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        }
        state.DepthClipEnable = TRUE;
        state.MultisampleEnable = FALSE; //TODO
        state.AntialiasedLineEnable = FALSE;
        state.ForcedSampleCount = 0;
        state.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }

    //Blend State
    {
        auto& blendState = pipelineInfo->_pipelineDesc._blendState;

        D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;

        ASSERT(sizeof(blendDesc.RenderTarget) / sizeof(blendDesc.RenderTarget[0]) == k_maxColorAttachments, "different max");
        memset(blendDesc.RenderTarget, 0, sizeof(blendDesc.RenderTarget));
        for (u32 i = 0; i < pipelineInfo->_renderpassDesc._desc._countColorAttachments; ++i)
        {
            blendDesc.RenderTarget[i].LogicOpEnable = blendState._logicalOpEnable;
            blendDesc.RenderTarget[i].LogicOp = D3DGraphicPipelineState::convertLogicOperationToD3D(blendState._logicalOp);

            blendDesc.RenderTarget[i].BlendEnable = blendState._colorBlendAttachments._colorBlendEnable;
            blendDesc.RenderTarget[i].BlendOp = D3DGraphicPipelineState::convertBlenOperationToD3D(blendState._colorBlendAttachments._blendOp);
            blendDesc.RenderTarget[i].BlendOpAlpha = D3DGraphicPipelineState::convertBlenOperationToD3D(blendState._colorBlendAttachments._alphaBlendOp);
            blendDesc.RenderTarget[i].SrcBlend = D3DGraphicPipelineState::convertBlendFacorToD3D(blendState._colorBlendAttachments._srcBlendFacor);
            blendDesc.RenderTarget[i].SrcBlendAlpha = D3DGraphicPipelineState::convertBlendFacorToD3D(blendState._colorBlendAttachments._srcAlphaBlendFacor);
            blendDesc.RenderTarget[i].DestBlend = D3DGraphicPipelineState::convertBlendFacorToD3D(blendState._colorBlendAttachments._dscBlendFacor);
            blendDesc.RenderTarget[i].DestBlendAlpha = D3DGraphicPipelineState::convertBlendFacorToD3D(blendState._colorBlendAttachments._dscAlphaBlendFacor);

            blendDesc.RenderTarget[i].RenderTargetWriteMask = static_cast<UINT8>(blendState._colorBlendAttachments._colorWriteMask);
        }
    }

    //Depthstencil State
    {
        auto& depthStencilState = pipelineInfo->_pipelineDesc._depthStencilState;

        CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc = {};
        depthStencilDesc.DepthEnable = depthStencilState._depthTestEnable;
        depthStencilDesc.DepthWriteMask = D3DGraphicPipelineState::convertWriteDepthToD3D(depthStencilState._depthWriteEnable);
        depthStencilDesc.DepthFunc = D3DGraphicPipelineState::convertDepthFunctionToD3D(depthStencilState._compareOp);

        //TODO
        depthStencilDesc.StencilEnable = depthStencilState._stencilTestEnable;
        depthStencilDesc.StencilReadMask = 0;
        depthStencilDesc.StencilWriteMask = 0;
        depthStencilDesc.FrontFace = {};
        depthStencilDesc.BackFace = {};

        depthStencilDesc.DepthBoundsTestEnable = FALSE;

        psoDesc.DepthStencilState = depthStencilDesc;
    }

    //Render Targets
    {
        D3D12_RT_FORMAT_ARRAY renderTargets = {};

        renderTargets.NumRenderTargets = pipelineInfo->_renderpassDesc._desc._countColorAttachments;
        for (u32 i = 0; i < pipelineInfo->_renderpassDesc._desc._countColorAttachments; ++i)
        {
            renderTargets.RTFormats[i] = D3DImage::convertImageFormatToD3DFormat(pipelineInfo->_renderpassDesc._desc._attachments[i]._format);
        }

        psoDesc.RTVFormats = renderTargets;

        if (pipelineInfo->_renderpassDesc._desc._hasDepthStencilAttahment)
        {
            psoDesc.DSVFormat = D3DImage::convertImageFormatToD3DFormat(pipelineInfo->_renderpassDesc._desc._attachments.back()._format);
        }
        else
        {
            psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        }
    }

    //Sample State
    {
        psoDesc.SampleMask = UINT_MAX;

        DXGI_SAMPLE_DESC sampleDesc = {};

        sampleDesc.Quality = 0;
        if (pipelineInfo->_renderpassDesc._desc._attachments[0]._samples > TextureSamples::TextureSamples_x1)
        {
            sampleDesc.Count = 1 << (u32)(pipelineInfo->_renderpassDesc._desc._attachments[0]._samples);
        }
        else
        {
            sampleDesc.Count = 1;
        }

        psoDesc.SampleDesc = sampleDesc;
    }

    //View instancing
    std::vector<D3D12_VIEW_INSTANCE_LOCATION> viewsIndexes;
    {
        u32 viewsMask = pipelineInfo->_renderpassDesc._desc._viewsMask;

        u32 countActiveViews = RenderPassDescription::countActiveViews(viewsMask);
        viewsIndexes.reserve(countActiveViews);

        CD3DX12_VIEW_INSTANCING_DESC viewInstancingDesc = {};
        if (viewsMask && D3DDeviceCaps::getInstance()->supportMultiview)
        {
            for (u32 i = 0; i < std::numeric_limits<u32>::digits; ++i)
            {
                if (RenderPassDescription::isActiveViewByIndex(viewsMask, i))
                {
                    viewsIndexes.push_back({ 0, i });
                }
            }

            viewInstancingDesc.Flags = D3D12_VIEW_INSTANCING_FLAG_ENABLE_VIEW_INSTANCE_MASKING;
            viewInstancingDesc.ViewInstanceCount =static_cast<UINT>(viewsIndexes.size());
            viewInstancingDesc.pViewInstanceLocations = viewsIndexes.data();
        }

        ASSERT(viewInstancingDesc.ViewInstanceCount <= D3D12_MAX_VIEW_INSTANCE_COUNT, "over max");
        psoDesc.ViewInstancingDesc = viewInstancingDesc;
    }

    //TODO
    psoDesc.CachedPSO = {};

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    streamDesc.SizeInBytes = sizeof(psoDesc);
    streamDesc.pPipelineStateSubobjectStream = &psoDesc;

    HRESULT result = m_device->CreatePipelineState(&streamDesc, DX_IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(result))
    {
        LOG_ERROR("D3DGraphicPipelineState::create CreateGraphicsPipelineState is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}

void D3DGraphicPipelineState::destroy()
{
    memset(m_bytecode, 0, sizeof(m_bytecode[toEnumType(ShaderType::Count)]));
    SAFE_DELETE(m_pipelineState);
}

bool D3DGraphicPipelineState::compileShader(const ShaderHeader* header, const void* source, u32 size)
{
    m_bytecode[toEnumType(header->_type)].BytecodeLength = static_cast<u32>(size);
    m_bytecode[toEnumType(header->_type)].pShaderBytecode = source;

    return true;
}

bool D3DGraphicPipelineState::separateSematic(const std::string& str, std::string& name, u32& id)
{
    std::string str_id("");

    bool stop = false;
    while (!stop)
    {
        for (u32 i = 0; i < 10; ++i)
        {
            size_t pos = str.find_last_of('0' + i);
            if (pos == std::string::npos)
            {
                name = str.substr(0, str.size() - str_id.size());

                std::reverse(str_id.begin(), str_id.end());
                id = std::strtoul(str_id.data(), 0, 10);
                return true;
            }

            str_id.append(1, str[pos]);
        }
    }
    return false;
}

bool D3DGraphicPipelineState::create(const PipelineComputeInfo* pipelineInfo)
{
    ASSERT(false, "must be graphic");
    return false;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
