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

    class RenderPipelineSelectionStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineSelectionStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineSelectionStage();

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

        void createRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame);

        scene::ModelHandler* m_modelHandler;

        renderer::RenderTargetState* m_renderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipelines;
        MaterialParameters m_parameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d