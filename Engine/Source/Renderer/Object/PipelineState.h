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
    *\n
    * Constructor param:
    const VertexInputAttribDescription& vertex
    const ShaderProgram* const program
    const RenderTargetState* const renderTaget
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

        PolygonMode getPolygonMode() const;
        FrontFace getFrontFace() const;
        CullMode getCullMode() const;

        void setDepthCompareOp(CompareOperation op);
        void setDepthTest(bool enable);
        void setDepthWrite(bool enable);

        CompareOperation getDepthCompareOp() const;
        bool isDepthTestEnable() const;
        bool isDepthWriteEnable() const;

        void setBlendEnable(bool enable, s32 attachment = -1);
        void setColorBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment = -1);
        void setColorBlendOp(BlendOperation op, s32 attachment = -1);
        void setAlphaBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment = -1);
        void setAlphaBlendOp(BlendOperation op, s32 attachment = -1);

        bool isBlendEnable(s32 attachment = -1) const;
        BlendFactor getSrcColorBlendFactor(s32 attachment = -1) const;
        BlendFactor getDstColorBlendFactor(s32 attachment = -1) const;
        BlendOperation getColorBlendOp(s32 attachment = -1) const;
        BlendFactor getSrcAlphaBlendFactor(s32 attachment = -1) const;
        BlendFactor getDstAlphaBlendFactor(s32 attachment = -1) const;
        BlendOperation getAlphaBlendOp(s32 attachment = -1) const;

        void setColorMask(ColorMaskFlags colormask, s32 attachment = -1);
        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        ColorMaskFlags getColorMask(s32 attachment = -1) const;
        PrimitiveTopology getPrimitiveTopology() const;

        const GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc() const;
        GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc();

    private:

        explicit GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept;
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept;

        void setShaderProgram(const ShaderProgram * program);
        void setRenderTaget(const RenderTargetState * target);

        void destroyPipelines(const std::vector<Pipeline*>& pipelines);

        friend CommandList;
        CommandList& m_cmdList;

        GraphicsPipelineStateDescription m_pipelineStateDesc;
        const ShaderProgram*             m_program;
        const RenderTargetState*         m_renderTaget;

        ObjectTracker<Pipeline> m_tracker;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
