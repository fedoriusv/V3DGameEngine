#pragma once

#include "Common.h"
#include "Context.h"
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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct PipelineGraphicInfo
    {
        ShaderProgram*                                   _program;
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

        bool createShader(const resource::Shader* shader)
        {
            if (!shader)
            {
                return false;
            }

            return compileShader(shader->getShaderHeader(), shader->m_source, shader->m_size);
        }

        virtual bool compileShader(const resource::ShaderHeader* header, const void* source, u32 size) = 0;

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
