#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
    class Texture2D;
    class RenderTargetState;
    class GraphicsPipelineState;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ModelHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineShadowStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineShadowStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineShadowStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialCascadeShadowsParameters
        {
            SHADER_PARAMETER(cb_ShadowBuffer);
        };

        struct MaterialScreenSpaceShadowsParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_CascadeShadowBuffer);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(s_ShadowSamplerState);
            SHADER_PARAMETER(t_TextureDepth);
            SHADER_PARAMETER(t_TextureNormals);
            SHADER_PARAMETER(t_CascadeShadows);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene);

        static void calculateShadowCascades(const scene::SceneData& data, const math::Vector3D& lightDirection, std::vector<math::Matrix4D>& lightSpaceMatrix, std::vector<f32>& cascadeSplits);

        scene::ModelHandler* const                m_modelHandler;
        renderer::CmdListRender*                  m_cmdList;

        std::vector<renderer::RenderTargetState*> m_cascadeRenderTargets;
        renderer::Texture2D*                      m_cascadeTextureArray;
        renderer::SamplerState*                   m_shadowSamplerState;
        renderer::GraphicsPipelineState*          m_cascadeShadowPipeline;
        MaterialCascadeShadowsParameters          m_cascadeShadowParameters;

        renderer::RenderTargetState*              m_SSShadowsRenderTarget;
        renderer::GraphicsPipelineState*          m_SSShadowsPipeline;
        MaterialScreenSpaceShadowsParameters      m_SSCascadeShadowParameters;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d