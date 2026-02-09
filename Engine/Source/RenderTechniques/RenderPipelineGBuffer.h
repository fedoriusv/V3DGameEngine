#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
    class RenderTargetState;
    class GraphicsPipelineState;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ModelHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineGBufferStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineGBufferStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineGBufferStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        struct MaterialParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(cb_Model);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(t_TextureAlbedo);
            SHADER_PARAMETER(t_TextureNormal);
            SHADER_PARAMETER(t_TextureMaterial);
            SHADER_PARAMETER(t_TextureMetalness);
            SHADER_PARAMETER(t_TextureRoughness);
            SHADER_PARAMETER(t_TextureHeight);
        };

        void createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        scene::ModelHandler* const                         m_modelHandler;

        renderer::RenderTargetState*                       m_GBufferRenderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipelines;
        std::vector<MaterialParameters>                    m_parameters;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d