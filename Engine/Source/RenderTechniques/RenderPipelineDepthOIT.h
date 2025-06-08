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

        renderer::RenderTargetState* m_stochasticTransparency_TotalAlpha;
        renderer::RenderTargetState* m_stochasticTransparency_Depth;
        renderer::RenderTargetState* m_stochasticTransparency_AccumulateColor;
        renderer::RenderTargetState* m_stochasticTransparency_Final;
        renderer::SamplerState* m_sampler;
        void executeStochasticTransparency(Device* device, scene::Scene::SceneData& state);
        

        renderer::RenderTargetState* m_transparencyRenderTarget;
        renderer::RenderTargetState* m_transparencyRenderTargetPass2;
        bool m_stochastic = true;

        enum Pass
        {
            StochasticTotalAlpha,
            StochasticDepth,
            StochasticTotalAccumulateColor,
            StochasticTotalFinal,
            Count
        };

        std::array<v3d::renderer::GraphicsPipelineState*, Pass::Count> m_pipeline = {};
    };

} //namespace renderer
} //namespace v3d