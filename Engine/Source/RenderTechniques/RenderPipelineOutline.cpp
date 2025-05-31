#include "RenderPipelineOutline.h"

namespace v3d
{
namespace renderer
{

RenderPipelineOutlineStage::RenderPipelineOutlineStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "outline")
{
}

RenderPipelineOutlineStage::~RenderPipelineOutlineStage()
{
}

void RenderPipelineOutlineStage::create(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineOutlineStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineOutlineStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineOutlineStage::execute(Device* device, scene::Scene::SceneData& state)
{
}

} //namespace renderer
} //namespace v3d