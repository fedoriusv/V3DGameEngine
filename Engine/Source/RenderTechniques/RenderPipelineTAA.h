#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineTAAStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineTAAStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineTAAStage();

        void create(Device* device, scene::SceneData& state) override;
        void destroy(Device* device, scene::SceneData& state) override;

        void prepare(Device* device, scene::SceneData& state) override;
        void execute(Device* device, scene::SceneData& state) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_renderTarget;
        renderer::GraphicsPipelineState* m_pipeline;
        renderer::SamplerState* m_samplerLinear;
        renderer::SamplerState* m_samplerPoint;
        renderer::Texture2D* m_resolved;
        renderer::Texture2D* m_history;

    };

} //namespace renderer
} //namespace v3d