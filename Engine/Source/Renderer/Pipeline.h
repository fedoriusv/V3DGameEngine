#pragma once

#include "Common.h"
#include "Context.h"
#include "Object/PipelineState.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct PipelineGraphicInfo
    {
        GraphicsPipelineState::GraphicsPipelineStateDesc _pipelineDesc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Pipeline : public utils::Observable
    {
    public:
        enum PipelineType
        {
            PipelineType_Graphic,
            PipelineType_Compute
        };

        Pipeline(PipelineType type) noexcept 
            : m_pipelineType(type)
        {
        };

        virtual ~Pipeline() {};

        virtual bool create(const PipelineGraphicInfo* pipelineInfo) = 0;
        virtual void destroy() = 0;

    protected:

        PipelineType m_pipelineType;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PipelineManager final
    {
    public:
        PipelineManager(Context* context)
            : m_context(context)
        {
        }
        
        ~PipelineManager()
        {
        }

        Pipeline* acquireGraphicPipeline()
        {
            //TODO:
        }

    private:

        Context*                    m_context;
        std::map<u32, Pipeline*>    m_pipelineGraphicList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
