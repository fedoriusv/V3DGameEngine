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

    class RenderPipelineTonemapStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineTonemapStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineTonemapStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Tonemapper);
            SHADER_PARAMETER(s_LinearMirrorSampler);
            SHADER_PARAMETER(s_LinearClampSampler);
            SHADER_PARAMETER(t_ColorTexture);
            SHADER_PARAMETER(t_LUTTexture);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& data);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_gammaRenderTarget;
        renderer::GraphicsPipelineState* m_pipeline;
        MaterialParameters m_parameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d