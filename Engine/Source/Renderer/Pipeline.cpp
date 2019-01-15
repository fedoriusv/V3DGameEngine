#include "Pipeline.h"

#include "Context.h"

namespace v3d
{
namespace renderer
{

Pipeline::Pipeline(PipelineType type) noexcept
    : m_pipelineType(type)
{
}

Pipeline::~Pipeline() 
{
}

bool Pipeline::createShader(const resource::Shader * shader)
{
    if (!shader)
    {
        return false;
    }

    return compileShader(shader->getShaderHeader(), shader->m_source, shader->m_size);
}

PipelineManager::PipelineManager(Context * context) noexcept
    : m_context(context)
{
}

PipelineManager::~PipelineManager()
{
}

Pipeline* PipelineManager::acquireGraphicPipeline(const RenderPass::RenderPassInfo& renderpassInfo)
{
    /*RenderPassDescription pDesc;
    pDesc._info = desc;

    Pipeline* pipeline = nullptr;
    auto found = m_pipelineGraphicList.emplace(pDesc._hash, pipeline);
    if (found.second)
    {
        pipeline = m_context->createPipeline(&pDesc._info);
        if (!pipeline->create())
        {
            pipeline->destroy();
            m_pipelineGraphicList.erase(pDesc._hash);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = pipeline;
        pipeline->registerNotify(this);

        return pipeline;
    }

    return found.first->second;*/

    return nullptr;
}

bool PipelineManager::removePipeline()
{
    return false;
}

void PipelineManager::clear()
{
}

} //namespace renderer
} //namespace v3d
