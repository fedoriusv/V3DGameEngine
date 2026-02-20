#include "RenderPipelineStage.h"
#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{

RenderPipelineStage::RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept
    : m_id(id)
    , m_renderTechnique(*technique)
    , m_enabled(true)
    , m_created(false)
{
    technique->addStage(m_id, this);
}

RenderPipelineStage::~RenderPipelineStage()
{
}

void RenderPipelineStage::onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event)
{
    //do nothing
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTechnique::RenderTechnique() noexcept
{
}

RenderTechnique::~RenderTechnique()
{
    for (auto& [id, stage] : m_stages)
    {
        delete stage;
    }
    m_stages.clear();
}

void RenderTechnique::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->create(device, scene, frame);
    }
}

void RenderTechnique::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->destroy(device, scene, frame);
    }
}

void RenderTechnique::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        if (stage->isEnabled())
        {
            stage->prepare(device, scene, frame);
        }
    }
}

void RenderTechnique::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        if (stage->isEnabled())
        {
            stage->execute(device, scene, frame);
        }
    }
}

void RenderTechnique::submit(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_batchJobs.empty()) //flush pending jobs
    {
        flushRenderJobs(device, m_batchJobs, scene);
        m_batchJobs.clear();
    }
       
    for (auto& [cmd, renderTask] : m_dependencyList)
    {
        renderTask->waitCompetition();
        device->submit(cmd);

        m_freeCmdList.push(cmd);

        delete renderTask;
    }

    m_dependencyList.clear();
}

void RenderTechnique::onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event)
{
    for (auto& [id, stage] : m_stages)
    {
        if (stage->isEnabled())
        {
            stage->onChanged(device, scene, event);
        }
    }
}

RenderPipelineStage* RenderTechnique::getStage(const std::string& id)
{
    auto found = std::find_if(m_stages.cbegin(), m_stages.cend(), [&id](const Stage& stage)
        {
            return stage._id == id;
        });

    if (found != m_stages.cend())
    {
        return found->_stage;
    }

    return nullptr;
}

void RenderTechnique::addStage(const std::string& id, RenderPipelineStage* stage)
{
    m_stages.emplace_back(id, stage);
}

renderer::CmdListRender* RenderTechnique::acquireCmdList(renderer::Device* device)
{
    if (!m_freeCmdList.empty())
    {
        renderer::CmdListRender* cmdList = m_freeCmdList.front();
        m_freeCmdList.pop();

        return cmdList;
    }

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    return cmdList;
}

} //namespace scene
} //namespace v3d