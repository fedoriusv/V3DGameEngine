#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineDepthOITStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineDepthOITStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineDepthOITStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        void executeStochasticTotalAlpha_Pass1(Device* device, scene::Scene::SceneData& state);
        void executeStochasticDepth_Pass2(Device* device, scene::Scene::SceneData& state);
        void executeStochasticAccumulateColor_Pass3(Device* device, scene::Scene::SceneData& state);
        void executeStochasticComposite_Pass4(Device* device, scene::Scene::SceneData& state);

        enum Pass
        {
            StochasticTotalAlpha,
            StochasticDepth,
            StochasticTotalAccumulateColor,
            StochasticComposite,
            Count
        };

        std::array<v3d::renderer::RenderTargetState*, Pass::Count> m_rt = {};
        std::array<v3d::renderer::GraphicsPipelineState*, Pass::Count> m_pipeline = {};
        renderer::SamplerState* m_sampler;
        renderer::UnorderedAccessTexture2D* m_depthSamples;
    };

} //namespace renderer
} //namespace v3d