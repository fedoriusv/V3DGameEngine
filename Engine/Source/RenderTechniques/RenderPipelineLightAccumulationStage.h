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

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Model);
            SHADER_PARAMETER(cb_Light);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_TextureBaseColor);
            SHADER_PARAMETER(t_TextureNormal);
            SHADER_PARAMETER(t_TextureMaterial);
            SHADER_PARAMETER(t_TextureDepth);
            SHADER_PARAMETER(t_TextureShadowmaps);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        scene::ModelHandler* m_modelHandler;
        renderer::RenderTargetState* m_lightRenderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipelines;
        std::vector<MaterialParameters> m_parameters;

        scene::Mesh* m_sphereVolume;
        scene::Mesh* m_coneVolume;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d