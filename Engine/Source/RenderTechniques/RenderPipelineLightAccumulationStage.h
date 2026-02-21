#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"
#include "RenderPipelineGBuffer.h"

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

    class ModelHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineLightAccumulationStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineLightAccumulationStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineLightAccumulationStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Model);
            SHADER_PARAMETER(cb_Light);
            SHADER_PARAMETER(cb_Shadow);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_TextureBaseColor);
            SHADER_PARAMETER(t_TextureNormal);
            SHADER_PARAMETER(t_TextureMaterial);
            SHADER_PARAMETER(t_TextureDepth);
            SHADER_PARAMETER(t_TextureShadowmaps);
            SHADER_PARAMETER(s_ShadowSamplerState);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene);

        void createPipelines(renderer::Device* device, scene::SceneData& scene);
        void destroyPipelines(renderer::Device* device, scene::SceneData& scene);

        scene::ModelHandler* const m_modelHandler;

        renderer::SamplerState* m_shadowSamplerState;
        renderer::RenderTargetState* m_lightRenderTarget;
        renderer::GraphicsPipelineState* m_pipeline;
        MaterialParameters m_parameters;

        scene::Mesh* m_sphereVolume;
        scene::Mesh* m_coneVolume;

        bool m_debugPunctualLightShadows;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d