#include "Pipeline.h"

#include "Context.h"
#include "Utils/Logger.h"
#include "Resource/Shader.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

Pipeline::Pipeline(PipelineType type) noexcept
    : m_key(0)
    , m_pipelineType(type)
{
}

Pipeline::~Pipeline() 
{
}

Pipeline::PipelineType Pipeline::getType() const
{
    return m_pipelineType;
}

bool Pipeline::createShader(const resource::Shader * shader)
{
    if (!shader)
    {
        return false;
    }

    return compileShader(&shader->getShaderHeader(), shader->m_source, shader->m_size);
}

PipelineManager::PipelineManager(Context * context) noexcept
    : m_context(context)
{
}

PipelineManager::~PipelineManager()
{
}

Pipeline* PipelineManager::acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    u32 hash = crc32c::Crc32c((u8*)&pipelineInfo->_pipelineDesc, sizeof(GraphicsPipelineStateDescription));
    hash = crc32c::Extend(hash, (u8*)&pipelineInfo->_renderpassDesc, sizeof(RenderPass::RenderPassInfo));

    u64 pipelineHash = 0;// pipelineInfo->_programDesc._hash;
    pipelineHash = pipelineHash << 8 | hash;

    Pipeline* pipeline = nullptr;
    auto found = m_pipelineGraphicList.emplace(pipelineHash, pipeline);
    if (found.second)
    {
        pipeline = m_context->createPipeline(Pipeline::PipelineType::PipelineType_Graphic);
        pipeline->m_key = pipelineHash;

        if (!pipeline->create(pipelineInfo))
        {
            pipeline->destroy();
            m_pipelineGraphicList.erase(pipelineHash);

            ASSERT(false, "can't create pipeline");
            return nullptr;
        }
        found.first->second = pipeline;
        pipeline->registerNotify(this);

        return pipeline;
    }

    return found.first->second;

    return nullptr;
}

bool PipelineManager::removePipeline(Pipeline* pipeline)
{
    if (pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic)
    {
        auto iter = m_pipelineGraphicList.find(pipeline->m_key);
        if (iter == m_pipelineGraphicList.cend())
        {
            LOG_DEBUG("PipelineManager pipeline not found");
            ASSERT(false, "pipeline");
            return false;
        }

        Pipeline* pipeline = iter->second;
        pipeline->notifyObservers();

        pipeline->destroy();
        delete  pipeline;

        return true;
    }

    return false;
}

void PipelineManager::clear()
{
    for (auto& pipeline : m_pipelineGraphicList)
    {
        pipeline.second->destroy();
        pipeline.second->notifyObservers();

        delete pipeline.second;
    }
    m_pipelineGraphicList.clear();
}

void PipelineManager::handleNotify(utils::Observable * ob)
{
    LOG_DEBUG("PipelineManager pipeline %x has been deleted", ob);
    m_pipelineGraphicList.erase(static_cast<Pipeline*>(ob)->m_key);
}

} //namespace renderer
} //namespace v3d
