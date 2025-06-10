#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineSOITStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineSOITStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineSOITStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_mrtPass;
        renderer::RenderTargetState* m_resolvePass;
        renderer::SamplerState* m_sampler;
        renderer::Texture2D* m_dummyTexture;

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