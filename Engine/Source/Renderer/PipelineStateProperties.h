#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{

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
    * GraphicsPipelineStateDescription
    */
    struct GraphicsPipelineStateDescription
    {
        struct RasterizationState
        {
            RasterizationState() 
                : _polygonMode(PolygonMode::PolygonMode_Triangle)
                , _frontFace(FrontFace::FrontFace_Clockwise)
                , _cullMode(CullMode::CullMode_Back)
            {
            }

            PolygonMode _polygonMode;
            FrontFace   _frontFace;
            CullMode    _cullMode;
        };

        struct BlendState
        {
            //TODO:
        };

        GraphicsPipelineStateDescription()
            : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
        {
        }

        RasterizationState  _rasterizationState;
        BlendState          _blendState;

        PrimitiveTopology   _primitiveTopology;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
