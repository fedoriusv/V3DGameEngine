#pragma once

#include "Common.h"

#include "RenderPipelineStage.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineCompositionStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineCompositionStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineCompositionStage();

        void create(Device* device, scene::Scene::SceneData& data) override;
        void destroy(Device* device, scene::Scene::SceneData& data) override;

        void prepare(Device* device, scene::Scene::SceneData& data) override;
        void execute(Device* device, scene::Scene::SceneData& data) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_compositionRenderTarget;
        renderer::SamplerState* m_sampler;
        std::vector<renderer::GraphicsPipelineState*> m_pipeline;
    };

} //namespace renderer
} //namespace v3d