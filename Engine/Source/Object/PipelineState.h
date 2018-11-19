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

    class GraphicsPipelineState : public PipelineState
    {
    public:
        GraphicsPipelineState() {};
        ~GraphicsPipelineState() {};

    private:

        struct RasterizationState
        {
            //TODO:
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

        GraphicsPipelineStateDesc m_pipelineStateDesc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
