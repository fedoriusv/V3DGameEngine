#pragma once

#include "Common.h"
#include "Object/PipelineState.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace resource
{
    struct ShaderHeader;
} //namespace resource
namespace renderer
{
    class Context;

    struct PipelineGraphicInfo
    {
        ShaderProgram*                                   _programDesc;
        GraphicsPipelineState::GraphicsPipelineStateDesc _pipelineDesc;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Pipeline base class. Render side
    */
    class Pipeline : public utils::Observable
    {
    public:
        enum PipelineType
        {
            PipelineType_Graphic,
            PipelineType_Compute
        };

        Pipeline(PipelineType type) noexcept;
        virtual ~Pipeline();

        virtual bool create(const PipelineGraphicInfo* pipelineInfo) = 0;
        virtual void destroy() = 0;

    protected:

        bool createShader(const resource::Shader* shader);
        virtual bool compileShader(const resource::ShaderHeader* header, const void* source, u32 size) = 0;

        PipelineType m_pipelineType;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PipelineManager class
    */
    class PipelineManager final
    {
    public:

        PipelineManager() = delete;

        PipelineManager(Context* context) noexcept;
        ~PipelineManager();

        Pipeline* acquireGraphicPipeline(const RenderPass::RenderPassInfo& renderpassInfo);
        bool removePipeline();
        void clear();

    private:

        Context*                    m_context;
        std::map<u32, Pipeline*>    m_pipelineGraphicList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
