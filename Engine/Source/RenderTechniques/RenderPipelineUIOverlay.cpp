#include "RenderPipelineUIOverlay.h"

namespace v3d
{
namespace renderer
{

RenderPipelineUIOverlayStage::RenderPipelineUIOverlayStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "ui")
{
}

RenderPipelineUIOverlayStage::~RenderPipelineUIOverlayStage()
{
}

void RenderPipelineUIOverlayStage::create(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::execute(Device* device, scene::Scene::SceneData& state)
{
}

} //namespace renderer
} //namespace v3d