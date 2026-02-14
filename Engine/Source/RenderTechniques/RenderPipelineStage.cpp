#include "RenderPipelineStage.h"
#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{

RenderPipelineStage::RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept
    : m_id(id)
    , m_renderTechnique(*technique)
{
    technique->addStage(m_id, this);
}

RenderPipelineStage::~RenderPipelineStage()
{
}

void RenderPipelineStage::onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event)
{
}

//void RenderPipelineStage::delayedDelete(v3d::Object* object, memory::MemoryLabel label)
//{
//    m_renderTechnique.m_delayedDeleteList.emplace_back(object, label);
//}

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
        stage->prepare(device, scene, frame);
    }
}

void RenderTechnique::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->execute(device, scene, frame);
    }
}

void RenderTechnique::submit(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
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
        stage->onChanged(device, scene, event);
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

void RenderTechnique::addRenderJob(renderer::Device* device, renderer::CmdListRender* cmds, task::TaskScheduler& worker, task::Task* renderTask)
{
    m_dependencyList.emplace_back(cmds, renderTask);
    worker.executeTask(renderTask, task::TaskPriority::Normal, task::TaskMask::WorkerThread);
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