#pragma once

#include "Common.h"
#include "ShaderProgram.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/CommandList.h"
#include "Renderer/ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PipelineState base class
    */
    class PipelineState : public Object
    {
    public:

        PipelineState() = default;
        virtual ~PipelineState() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * GraphicsPipelineState class
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

        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        const GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc() const;
        GraphicsPipelineStateDescription& getGraphicsPipelineStateDesc();

    private:

        explicit GraphicsPipelineState(CommandList& cmdList, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;
        explicit GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept;

        void setShaderProgram(const ShaderProgram * program);
        void setRenderTaget(const RenderTarget * target);

        void destroyPipelines(const std::vector<Pipeline*>& pipelines);

        friend CommandList;
        CommandList&                     m_cmdList;

        GraphicsPipelineStateDescription m_pipelineStateDesc;
        const ShaderProgram*             m_program;
        const RenderTarget*              m_renderTaget;


        template<class TRenderObject>
        static void objectTrackerCallback(Object* self, const std::vector<TRenderObject*>& objects);
        
        ObjectTracker<Pipeline>         m_tracker;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TRenderObject>
    inline void GraphicsPipelineState::objectTrackerCallback(Object* self, const std::vector<TRenderObject*>& objects)
    {
        static_cast<GraphicsPipelineState*>(self)->destroyPipelines(objects);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
