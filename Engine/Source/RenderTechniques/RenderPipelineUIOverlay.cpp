#include "RenderPipelineUIOverlay.h"

namespace v3d
{
namespace renderer
{

RenderPipelineUIOverlayStage::RenderPipelineUIOverlayStage(RenderTechnique* technique, ui::WidgetHandler* uiHandler) noexcept
    : RenderPipelineStage(technique, "ui")
{
}

RenderPipelineUIOverlayStage::~RenderPipelineUIOverlayStage()
{
}

void RenderPipelineUIOverlayStage::create(Device* device, scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::destroy(Device* device, scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::prepare(Device* device, scene::SceneData& state)
{
}

void RenderPipelineUIOverlayStage::execute(Device* device, scene::SceneData& state)
{
}

} //namespace renderer
} //namespace v3d