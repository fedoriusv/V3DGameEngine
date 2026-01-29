#include "RenderPipelineStage.h"

namespace v3d
{
namespace scene
{

static const u32 k_countRenderTasks = 4;

RenderPipelineStage::RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept
    : m_id(id)
    , m_renderTechnique(*technique)
{
    technique->addStage(m_id, this);
}

RenderPipelineStage::~RenderPipelineStage()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTechnique::RenderTechnique() noexcept
    : m_renderWorker(k_countRenderTasks)
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

    m_renderWorker.mainThreadLoop();
}

void RenderTechnique::submit(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [cmd, renderTask] : m_dependencyList)
    {
        renderTask->waitCompetition();
        device->submit(cmd);

        delete renderTask;
    }

    m_dependencyList.clear();
}

void RenderTechnique::addStage(const std::string& id, RenderPipelineStage* stage)
{
    m_stages.emplace_back(id, stage);
}

void RenderTechnique::addRenderJob(renderer::Device* device, renderer::CmdList* cmd, task::Task* renderTask)
{
#if 0 //Debug cmds
    m_renderWorker.executeTask(renderTask, task::TaskPriority::Normal, task::TaskMask::MainThread);
    m_renderWorker.mainThreadLoop();

    device->submit(cmd, true);
#else
    m_dependencyList.emplace_back(cmd, renderTask);
    m_renderWorker.executeTask(renderTask, task::TaskPriority::Normal, task::TaskMask::WorkerThread);
#endif
}

} //namespace scene
} //namespace v3d