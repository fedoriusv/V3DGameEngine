#include "Pipeline.h"

#include "Context.h"
#include "Utils/Logger.h"
#include "Shader.h"

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

bool Pipeline::createShader(const Shader* shader)
{
    if (!shader)
    {
        return false;
    }

    return compileShader(&shader->getShaderHeader(), shader->m_source, shader->m_size);
}

bool Pipeline::createProgram(const ShaderProgramDescription& desc)
{
    std::vector<std::tuple<const ShaderHeader*, const void*, u32>> shadersData;
    for (u32 type = ShaderType::ShaderType_Vertex; type < ShaderType_Count; ++type)
    {
        const Shader* shader = desc._shaders[type];
        if (!shader)
        {
            continue;
        }

        shadersData.push_back({ &shader->getShaderHeader(), shader->m_source, shader->m_size });
    }

    return compileShaders(shadersData);
}

bool Pipeline::compileShader(const ShaderHeader* header, const void* source, u32 size)
{
    return false;
}

bool Pipeline::compileShaders(std::vector<std::tuple<const ShaderHeader*, const void*, u32>>& shaders)
{
    return false;
}

PipelineManager::PipelineManager(Context * context) noexcept
    : m_context(context)
{
}

PipelineManager::~PipelineManager()
{
    PipelineManager::clear();
}

Pipeline* PipelineManager::acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo& pipelineInfo)
{
    u32 hash = crc32c::Crc32c((u8*)&pipelineInfo._pipelineDesc, sizeof(GraphicsPipelineStateDescription));
    hash = crc32c::Extend(hash, (u8*)&pipelineInfo._renderpassDesc, sizeof(RenderPass::RenderPassInfo));

    u64 pipelineHash = pipelineInfo._programDesc._hash;
    pipelineHash = hash | pipelineHash << 32;

    Pipeline* pipeline = nullptr;
    auto found = m_pipelineGraphicList.emplace(pipelineHash, pipeline);
    if (found.second)
    {
        pipeline = m_context->createPipeline(Pipeline::PipelineType::PipelineType_Graphic);
        pipeline->m_key = pipelineHash;

        if (!pipeline->create(&pipelineInfo))
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
}

bool PipelineManager::removePipeline(Pipeline* pipeLine)
{
    if (pipeLine->getType() == Pipeline::PipelineType::PipelineType_Graphic)
    {
        auto iter = m_pipelineGraphicList.find(pipeLine->m_key);
        if (iter == m_pipelineGraphicList.cend())
        {
            LOG_DEBUG("PipelineManager pipeline not found");
            ASSERT(false, "pipeline");
            return false;
        }

        Pipeline* pipeline = iter->second;
        ASSERT(pipeline == pipeLine, "Different pointers");
        if (pipeline->linked())
        {
            LOG_WARNING("PipelineManager::removePipeline pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
            //return false;
        }
        m_pipelineGraphicList.erase(iter);

        pipeline->notifyObservers();

        pipeline->destroy();
        delete  pipeline;

        return true;
    }

    return false;
}

void PipelineManager::clear()
{
    for (auto& iter : m_pipelineGraphicList)
    {
        Pipeline* pipeline = iter.second;
        if (pipeline->linked())
        {
            LOG_WARNING("PipelineManager::clear pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
        }
        pipeline->notifyObservers();

        pipeline->destroy();
        delete pipeline;
    }
    m_pipelineGraphicList.clear();
}

void PipelineManager::handleNotify(const utils::Observable * ob)
{
    LOG_DEBUG("PipelineManager pipeline %x has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
