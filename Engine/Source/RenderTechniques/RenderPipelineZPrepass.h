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
} //namespace scene
namespace renderer
{
    class RenderPipelineZPrepassStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineZPrepassStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineZPrepassStage();

        void create(Device* device, scene::SceneData& state) override;
        void destroy(Device* device, scene::SceneData& state) override;

        void prepare(Device* device, scene::SceneData& state) override;
        void execute(Device* device, scene::SceneData& state) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_depthRenderTarget;
        v3d::renderer::GraphicsPipelineState* m_depthPipeline;
    };

} //namespace renderer
} //namespace v3d