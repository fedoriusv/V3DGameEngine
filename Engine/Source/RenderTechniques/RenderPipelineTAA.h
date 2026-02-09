#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class RenderTargetState;
    class GraphicsPipelineState;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineTAAStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineTAAStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineTAAStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(s_SamplerLinear);
            SHADER_PARAMETER(s_SamplerPoint);
            SHADER_PARAMETER(t_TextureBaseColor);
            SHADER_PARAMETER(t_TextureHistory);
            SHADER_PARAMETER(t_TextureVelocity);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame);

        renderer::RenderTargetState* m_renderTarget;
        renderer::GraphicsPipelineState* m_pipeline;
        MaterialParameters m_parameters;

        renderer::Texture2D* m_resolvedTexture;
        renderer::Texture2D* m_historyTexture;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d