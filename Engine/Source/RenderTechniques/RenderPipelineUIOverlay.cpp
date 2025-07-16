#include "RenderPipelineUIOverlay.h"

namespace v3d
{
namespace renderer
{

RenderPipelineUIOverlayStage::RenderPipelineUIOverlayStage(RenderTechnique* technique, ui::WidgetHandler* uiHandler) noexcept
    : RenderPipelineStage(technique, "Overlay")
{
}

RenderPipelineUIOverlayStage::~RenderPipelineUIOverlayStage()
{
}

void RenderPipelineUIOverlayStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

} //namespace renderer
} //namespace v3d