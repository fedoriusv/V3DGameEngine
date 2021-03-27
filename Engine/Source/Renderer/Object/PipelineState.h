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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PipelineState base class. Game side
    */
    class PipelineState : public Object
    {
    public:

        PipelineState() = delete;
        PipelineState(const PipelineState&) = delete;

        explicit PipelineState(CommandList& cmdList) noexcept;
        virtual ~PipelineState();

    protected:

        void destroyPipelines(const std::vector<Pipeline*>& pipelines);

        friend CommandList;
        CommandList& m_cmdList;

        ObjectTracker<Pipeline> m_tracker;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Cover all atttachments
    */
    const s32 k_allAttachments = -1;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GraphicsPipelineState class. Game side.
    * Includes a shader program and render states
    */
    class GraphicsPipelineState : public PipelineState
    {
    public:

        GraphicsPipelineState() = delete;
        GraphicsPipelineState(const GraphicsPipelineState&) = delete;
        ~GraphicsPipelineState() = default;

        void setPolygonMode(PolygonMode polygonMode);
        void setFrontFace(FrontFace frontFace);
        void setCullMode(CullMode cullMode);

        void setDepthBias(f32 constantFactor, f32 clamp, f32 slopeFactor);

        PolygonMode getPolygonMode() const;
        FrontFace getFrontFace() const;
        CullMode getCullMode() const;

        void setDepthCompareOp(CompareOperation op);
        void setDepthTest(bool enable);
        void setDepthWrite(bool enable);

        CompareOperation getDepthCompareOp() const;
        bool isDepthTestEnable() const;
        bool isDepthWriteEnable() const;

        void setBlendEnable(bool enable, s32 attachment = k_allAttachments);
        void setColorBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment = k_allAttachments);
        void setColorBlendOp(BlendOperation op, s32 attachment = k_allAttachments);
        void setAlphaBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment = k_allAttachments);
        void setAlphaBlendOp(BlendOperation op, s32 attachment = k_allAttachments);

        bool isBlendEnable(s32 attachment = k_allAttachments) const;
        BlendFactor getSrcColorBlendFactor(s32 attachment = k_allAttachments) const;
        BlendFactor getDstColorBlendFactor(s32 attachment = k_allAttachments) const;
        BlendOperation getColorBlendOp(s32 attachment = k_allAttachments) const;
        BlendFactor getSrcAlphaBlendFactor(s32 attachment = k_allAttachments) const;
        BlendFactor getDstAlphaBlendFactor(s32 attachment = k_allAttachments) const;
        BlendOperation getAlphaBlendOp(s32 attachment = k_allAttachments) const;

        void setColorMask(ColorMaskFlags colormask, s32 attachment = k_allAttachments);
        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        ColorMaskFlags getColorMask(s32 attachment = k_allAttachments) const;
        PrimitiveTopology getPrimitiveTopology() const;

        const GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc() const;
        GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc();

    private:

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        * 
        * @param const VertexInputAttribDescription& vertex [required]
        * @param const ShaderProgram* const program [required]
        * @param onst RenderTargetState* const renderTaget [required]
        */
        explicit GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept;

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const GraphicsPipelineStateDescription& desc [required]
        * @param const ShaderProgram* const program [required]
        * @param onst RenderTargetState* const renderTaget [required]
        */
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept;

        void setShaderProgram(const ShaderProgram* program);
        void setRenderTaget(const RenderTargetState* target);

        friend CommandList;

        GraphicsPipelineStateDescription m_pipelineStateDesc;
        const ShaderProgram*             m_program;
        const RenderTargetState*         m_renderTaget;
    };

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
