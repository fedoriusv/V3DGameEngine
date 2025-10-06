#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"
#include "RenderPipelineGBuffer.h"

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

    class RenderPipelineDeferredLightingStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineDeferredLightingStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineDeferredLightingStage();

        void create(renderer::Device * device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Light);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_TextureBaseColor);
            SHADER_PARAMETER(t_TextureNormal);
            SHADER_PARAMETER(t_TextureMaterial);
            SHADER_PARAMETER(t_TextureDepth);
#if WORLD_POS_ATTACHMENT
            SHADER_PARAMETER(t_TextureWorldPos);
#endif
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& data);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_deferredRenderTarget;
        v3d::renderer::GraphicsPipelineState* m_pipeline;
        MaterialParameters m_parameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d