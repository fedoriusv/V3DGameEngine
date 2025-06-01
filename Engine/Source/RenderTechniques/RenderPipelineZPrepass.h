#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineZPrepassStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineZPrepassStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineZPrepassStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_depthRenderTarget;
        v3d::renderer::GraphicsPipelineState* m_depthPipeline;
    };

} //namespace renderer
} //namespace v3d