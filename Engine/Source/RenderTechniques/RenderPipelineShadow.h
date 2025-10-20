#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class Device;
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

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_ShadowBuffer);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene);

        static void calculateShadowCascades(const scene::SceneData& data, const math::Vector3D& lightDirection, LightNodeEntry& entry);

        scene::ModelHandler* const m_modelHandler;

        std::vector<renderer::RenderTargetState*> m_cascadeRenderTargets;
        renderer::Texture2D* m_cascadeTextureArray;

        renderer::GraphicsPipelineState* m_pipeline;
        MaterialParameters m_parameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d