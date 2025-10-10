#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

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

    class RenderPipelineSkyboxStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineSkyboxStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineSkyboxStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_SkyboxTexture);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& data);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data);

        scene::ModelHandler* m_modelHandler;
        renderer::RenderTargetState* m_renderTarget;
        std::vector<renderer::GraphicsPipelineState*> m_pipelines;
        std::vector<scene::Mesh*> m_volumes;
        MaterialParameters m_parameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d