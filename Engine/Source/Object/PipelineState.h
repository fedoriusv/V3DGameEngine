#pragma once

#include "Common.h"
#include "CommandList.h"

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
            PolygonMode _polygonMode;
            FrontFace   _frontFace;
            CullMode    _cullMode;
        };

        struct BlendStateDesc
        {
            //TODO:
        };

        struct GraphicsPipelineStateDesc
        {
            RasterizationState  _rasterizationState;
            BlendStateDesc      _blendState;

        };

        GraphicsPipelineState() noexcept;
        GraphicsPipelineState(const GraphicsPipelineStateDesc& desc) noexcept;

        ~GraphicsPipelineState();

        void setPolygonMode(PolygonMode polygonMode);

    private:

        GraphicsPipelineStateDesc m_pipelineStateDesc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
