#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{

RenderPipelineStage::RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept
    : m_id(id)
{
    technique->addStage(m_id, this);
}

RenderPipelineStage::~RenderPipelineStage()
{
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

void RenderTechnique::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->create(device, scene, frame);
    }
}

void RenderTechnique::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->destroy(device, scene, frame);
    }
}

void RenderTechnique::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->prepare(device, scene, frame);
    }
}

void RenderTechnique::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->execute(device, scene, frame);
    }
}

void RenderTechnique::addStage(const std::string& id, RenderPipelineStage* stage)
{
    m_stages.emplace_back(id, stage);
}

} //namespace renderer
} //namespace v3d