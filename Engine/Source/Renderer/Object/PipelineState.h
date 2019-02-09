#pragma once

#include "Common.h"
#include "Object.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PipelineState base class. Game side
    */
    class PipelineState : public Object
    {
    public:

        PipelineState() = default;
        virtual ~PipelineState() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * GraphicsPipelineState class. Game side
    */
    class GraphicsPipelineState : public PipelineState
    {
    public:

        GraphicsPipelineState() = delete;
        GraphicsPipelineState(const GraphicsPipelineState&) = delete;
        ~GraphicsPipelineState();

        void setPolygonMode(PolygonMode polygonMode);
        void setFrontFace(FrontFace frontFace);
        void setCullMode(CullMode cullMode);
        void setDepthCompareOp(CompareOperation op);
        void setDepthTest(bool enable);
        void setDepthWrite(bool enable);

        void setColorMask(ColorMaskFlags colormask, s32 attachmet = -1);

        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        const GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc() const;
        GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc();

    private:

        explicit GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;

        void setShaderProgram(const ShaderProgram * program);
        void setRenderTaget(const RenderTarget * target);

        void destroyPipelines(const std::vector<Pipeline*>& pipelines);

        friend CommandList;
        CommandList& m_cmdList;

        GraphicsPipelineStateDescription m_pipelineStateDesc;
        const ShaderProgram*             m_program;
        const RenderTarget*              m_renderTaget;

        ObjectTracker<Pipeline> m_tracker;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
