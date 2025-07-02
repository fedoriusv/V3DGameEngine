#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace scene
{
    class ModelHandler;
} // namespace scene
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineZPrepassStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineZPrepassStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineZPrepassStage();

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& scene);
        void destroyRenderTarget(Device* device, scene::SceneData& scene);

        scene::ModelHandler* m_modelHandler;

        renderer::RenderTargetState* m_depthRenderTarget;
        v3d::renderer::GraphicsPipelineState* m_depthPipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace renderer
} // namespace v3d