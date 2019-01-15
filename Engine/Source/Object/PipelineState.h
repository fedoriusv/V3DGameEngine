#pragma once

#include "Common.h"
#include "ShaderProgram.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PipelineState : public Object 
    {
    public:

        PipelineState() = default;
        virtual ~PipelineState() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class PolygonMode : u32
    {
        PolygonMode_Triangle,
        PolygonMode_Line,
        PolygonMode_Point
    };

    enum class PrimitiveTopology : u32
    {
        PrimitiveTopology_PointList,
        PrimitiveTopology_LineList,
        PrimitiveTopology_LineStrip,
        PrimitiveTopology_TriangleList,
        PrimitiveTopology_TriangleStrip,
        PrimitiveTopology_TriangleFan,
        PrimitiveTopology_LineListWithAdjacency,
        PrimitiveTopology_LineStripWithAdjacency,
        PrimitiveTopology_TriangleListWithAdjacency,
        PrimitiveTopology_TriangleStripWithAdjacency,
        PrimitiveTopology_PatchList
    };

    enum class FrontFace : u32
    {
        FrontFace_CounterClockwise,
        FrontFace_Clockwise
    };

    enum class CullMode : u32
    {
        CullMode_None,
        CullMode_Front,
        CullMode_Back,
        CullMode_FrontAndBack,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * GraphicsPipelineState class
    */
    class GraphicsPipelineState : public PipelineState
    {
    public:

        struct RasterizationState
        {
            RasterizationState();

            PolygonMode _polygonMode;
            FrontFace   _frontFace;
            CullMode    _cullMode;
        };

        struct BlendState
        {
            //TODO:
        };

        struct ShaderStage
        {
        };

        struct GraphicsPipelineStateDesc
        {
            GraphicsPipelineStateDesc();

            RasterizationState  _rasterizationState;
            BlendState          _blendState;
            ShaderStage         _shaderStage;

            PrimitiveTopology   _primitiveTopology;

        };

        GraphicsPipelineState() = delete;
        ~GraphicsPipelineState();

        void setPolygonMode(PolygonMode polygonMode);
        void setFrontFace(FrontFace frontFace);
        void setCullMode(CullMode cullMode);

        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        const GraphicsPipelineStateDesc& getGraphicsPipelineStateDesc() const;
        GraphicsPipelineStateDesc& getGraphicsPipelineStateDesc();

    private:

        explicit GraphicsPipelineState(CommandList& cmdList, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDesc& desc, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;

        void setShaderProgram(const ShaderProgram * program);
        void setRenderTaget(const RenderTarget * target);

        GraphicsPipelineStateDesc m_pipelineStateDesc;
        const ShaderProgram*      m_program;
        const RenderTarget*       m_renderTaget;

        CommandList&              m_cmdList;

        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
