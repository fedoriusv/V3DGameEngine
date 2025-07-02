#include "RenderPipelineLightingStage.h"

namespace v3d
{
namespace renderer
{

RenderPipelineLightingStage::RenderPipelineLightingStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Light")
    , m_modelHandler(modelHandler)
{
}

RenderPipelineLightingStage::~RenderPipelineLightingStage()
{
}

void RenderPipelineLightingStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineLightingStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineLightingStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

void RenderPipelineLightingStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
}

} // namespace renderer
} // namespace v3d