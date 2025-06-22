#pragma once

#include "Common.h"

#include "RenderPipelineStage.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineGammaCorrectionStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineGammaCorrectionStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineGammaCorrectionStage();

        void create(Device* device, scene::SceneData& state) override;
        void destroy(Device* device, scene::SceneData& state) override;

        void prepare(Device* device, scene::SceneData& state) override;
        void execute(Device* device, scene::SceneData& state) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_gammaRenderTarget;
        renderer::SamplerState* m_sampler;
        std::vector<renderer::GraphicsPipelineState*> m_pipeline;
    };

} //namespace renderer
} //namespace v3d