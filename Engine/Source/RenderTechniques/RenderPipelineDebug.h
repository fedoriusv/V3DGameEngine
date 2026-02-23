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

    class RenderPipelineDebugStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineDebugStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineDebugStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Model);
        };

        struct DebugVisualizerMaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Visualizer);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_RenderTargetTexture);
            SHADER_PARAMETER(t_VisualizeTexture);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        scene::ModelHandler* m_modelHandler;
        renderer::RenderTargetState* m_renderTarget;
        std::vector<renderer::GraphicsPipelineState*> m_pipelines;
        std::vector<MaterialParameters> m_parameters;

        renderer::GraphicsPipelineState* m_debugVisualizerPipeline;
        renderer::Texture2D* m_debugVisualizerTexture;
        DebugVisualizerMaterialParameters m_debugVisualizerParameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d