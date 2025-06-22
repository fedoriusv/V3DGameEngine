#pragma once

#include "Common.h"

#include "RenderPipelineStage.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineFXAAStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineFXAAStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineFXAAStage();

        void create(Device* device, scene::SceneData& data) override;
        void destroy(Device* device, scene::SceneData& data) override;

        void prepare(Device* device, scene::SceneData& data) override;
        void execute(Device* device, scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_renderTarget;
        renderer::SamplerState* m_sampler;
        std::vector<renderer::GraphicsPipelineState*> m_pipeline;
    };

} //namespace renderer
} //namespace v3d