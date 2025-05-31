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

void RenderTechnique::create(Device* device, scene::Scene::SceneData& data)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->create(device, data);
    }
}

void RenderTechnique::destroy(Device* device, scene::Scene::SceneData& data)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->destroy(device, data);
    }
}

void RenderTechnique::prepare(Device* device, scene::Scene::SceneData& data)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->prepare(device, data);
    }
}

void RenderTechnique::execute(Device* device, scene::Scene::SceneData& data)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->execute(device, data);
    }
}
void RenderTechnique::changed(Device* device, scene::Scene::SceneData& data)
{
    for (auto& [id, stage] : m_stages)
    {
        stage->changed(device, data);
    }
}

void RenderTechnique::addStage(const std::string& id, RenderPipelineStage* data)
{
    m_stages.emplace_back(id, data);
}



} //namespace renderer
} //namespace v3d