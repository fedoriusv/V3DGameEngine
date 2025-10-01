#include "RenderPipelineUIOverlay.h"

namespace v3d
{
namespace scene
{

RenderPipelineUIOverlayStage::RenderPipelineUIOverlayStage(RenderTechnique* technique, ui::WidgetHandler* uiHandler) noexcept
    : RenderPipelineStage(technique, "Overlay")
{
}

RenderPipelineUIOverlayStage::~RenderPipelineUIOverlayStage()
{
}

void RenderPipelineUIOverlayStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineUIOverlayStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

} //namespace scene
} //namespace v3d