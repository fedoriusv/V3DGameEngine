#pragma once

#include "Common.h"
#include "CommandList.h"
#include "ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PipelineState : public Object {};

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

        GraphicsPipelineState() noexcept;
        explicit GraphicsPipelineState(const GraphicsPipelineStateDesc& desc) noexcept;

        ~GraphicsPipelineState();

        void setPolygonMode(PolygonMode polygonMode);
        void setFrontFace(FrontFace frontFace);
        void setCullMode(CullMode cullMode);

        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

    private:

        GraphicsPipelineStateDesc m_pipelineStateDesc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
