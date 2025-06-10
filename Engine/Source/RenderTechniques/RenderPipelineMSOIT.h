#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineMSOITStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineMSOITStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineMSOITStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_msaaPass;
        renderer::RenderTargetState* m_resolvePass;
        renderer::SamplerState* m_sampler;

        enum Pass
        {
            MultiSamplePass,
            ResolvePass,

            Count
        };

        std::array<v3d::renderer::GraphicsPipelineState*, Pass::Count> m_pipeline = {};
    };

} //namespace renderer
} //namespace v3d