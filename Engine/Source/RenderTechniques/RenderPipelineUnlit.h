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

    class RenderPipelineUnlitStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineUnlitStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineUnlitStage();

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        scene::ModelHandler* m_modelHandler;

        renderer::RenderTargetState* m_renderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipelines;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace renderer
} // namespace v3d