#pragma once

#include "Common.h"
#include "Object.h"

#include "PipelineStateProperties.h"
#include "BufferProperties.h"
#include "ObjectTracker.h"
#include "CommandList.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Pipeline;
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PipelineState base class. Game side
    */
    class PipelineState : public Object
    {
    public:

        explicit PipelineState(CommandList& cmdList, [[maybe_unused]] const std::string& name = "") noexcept;
        virtual ~PipelineState();

    protected:

        void destroyPipelines(const std::vector<Pipeline*>& pipelines);

        PipelineState() = delete;
        PipelineState(const PipelineState&) = delete;

        friend CommandList;
        CommandList& m_cmdList;

        ObjectTracker<Pipeline> m_tracker;
        [[maybe_unused]] const std::string m_name;
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

        /**
        * @brief GraphicsPipelineState destructor
        */
        ~GraphicsPipelineState() = default;

        /**
        * @brief setPolygonMode method. Rasterization state
        * @param PolygonMode polygonMode [required]
        */
        void setPolygonMode(PolygonMode polygonMode);

        /**
        * @brief setFrontFace method. Rasterization state
        * @param FrontFace frontFace [required]
        */
        void setFrontFace(FrontFace frontFace);

        /**
        * @brief setCullMode method. Rasterization state
        * @param CullMode cullMode [required]
        */
        void setCullMode(CullMode cullMode);

        /**
        * @brief setDepthBias method. Rasterization state
        * @param f32 constantFactor [required]
        * @param f32 clamp [required]
        * @param f32 slopeFactor [required]
        */
        void setDepthBias(f32 constantFactor, f32 clamp, f32 slopeFactor);

        /**
        * @brief setDisacardRasterization method. Rasterization state
        * @param bool disabled [required]
        */
        void setDisacardRasterization(bool disabled);

        /**
        * @brief getPolygonMode method. Rasterization state
        * @return PolygonMode
        */
        PolygonMode getPolygonMode() const;

        /**
        * @brief getFrontFace method. Rasterization state
        * @return FrontFace
        */
        FrontFace getFrontFace() const;

        /**
        * @brief getCullMode method. Rasterization state
        * @return CullMode
        */
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
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const GraphicsPipelineStateDescription& desc [required]
        * @param const ShaderProgram* const program [required]
        * @param const RenderTargetState* const renderTaget [required]
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget, [[maybe_unused]] const std::string& name = "") noexcept;

        GraphicsPipelineState() = delete;
        GraphicsPipelineState(const GraphicsPipelineState&) = delete;

        void setShaderProgram(const ShaderProgram* program);
        void setRenderTaget(const RenderTargetState* target);

        friend CommandList;

        GraphicsPipelineStateDescription m_pipelineStateDesc;
        const ShaderProgram*             m_program;
        const RenderTargetState*         m_renderTaget;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ComputePipelineState class. Game side.
    * Includes a shader program
    */
    class ComputePipelineState : public PipelineState
    {
    public:

        /**
        * @brief ComputePipelineState destructor
        */
        ~ComputePipelineState() = default;

        /**
        * @brief ComputePipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const ShaderProgram* const program [required]
        * @param const std::string& name [optional]
        */
        explicit ComputePipelineState(CommandList& cmdList, const ShaderProgram* const program, [[maybe_unused]] const std::string& name = "") noexcept;

        ComputePipelineState() = delete;
        ComputePipelineState(const ComputePipelineState&) = delete;

        friend CommandList;

        const ShaderProgram* m_program;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
