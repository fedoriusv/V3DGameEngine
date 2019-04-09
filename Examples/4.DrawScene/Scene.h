#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Utils/IntrusivePointer.h"

namespace v3d
{
namespace renderer
{
    class GraphicsPipelineState;
    class SamplerState;
    class ShaderProgram;
    class Texture2D;

} //namespace renderer
namespace scene
{
    class Camera;
    class ModelHelper;
    class MaterialHelper;

    class Scene
    {
    public:
        Scene(const core::Dimension2D& size) noexcept;
        ~Scene();

        void onLoad(v3d::renderer::CommandList & cmd);

        void onUpdate();
        void onRender(v3d::renderer::CommandList& cmd);

        void setCamera(scene::Camera* camera);
        void setCameraPosition(const core::Vector3D& position);

    private:

        renderer::RenderTargetState* m_simpleBackbuffer;
        renderer::ShaderProgram* m_simpleProgram;
        renderer::GraphicsPipelineState* m_simplePipeline;
        utils::IntrusivePointer<renderer::VertexStreamBuffer> m_ScreenQuad;

        //void updateProgramParameters(renderer::ShaderProgram* program, scene::ModelHelper* model, scene::MaterialHelper* material);


        std::vector<scene::MaterialHelper*> m_sponzaMaterials;
        scene::ModelHelper* m_modelDrawer;


        core::Dimension2D m_size;
        core::Vector3D    m_viewPosition;
        scene::Camera*    m_camera;

        template<u32 attachments>
        struct RenderPass
        {
            std::array< utils::IntrusivePointer<renderer::Texture2D>, attachments> colorTexture;
            utils::IntrusivePointer<renderer::Texture2D>                           depthTexture;

            utils::IntrusivePointer<renderer::RenderTargetState>                   renderTarget;
        };

        utils::IntrusivePointer<renderer::SamplerState> m_Sampler;
        utils::IntrusivePointer<renderer::Texture2D> m_DummyTexture;

        //Pass 1
        RenderPass<3> m_MRTRenderPass;
        renderer::GraphicsPipelineState* m_MRTOpaquePipeline;
        renderer::ShaderProgram*         m_MRTOpaqueProgram;

        renderer::GraphicsPipelineState* m_MRTTransparentPipeline;
        renderer::ShaderProgram*         m_MRTTransparentProgram;

        //renderer::GraphicsPipelineState* m_MRTParticlesPipeline;
        //renderer::GraphicsPipelineState* m_mptSkyboxPipeline;

        //Pass 2
        RenderPass<1> m_SSAORenderPass;
        renderer::GraphicsPipelineState* m_SSAOPipeline;
        renderer::ShaderProgram*         m_SSAOProgram;

        std::vector<core::Vector4D> m_SSAOKernel;
        utils::IntrusivePointer<renderer::Texture2D> m_SSAONoiseTexture;
        utils::IntrusivePointer<renderer::SamplerState> m_SSAONoiseSampler;

        //Pass 3
        RenderPass<1> m_SSAOBlurRenderPass;
        renderer::GraphicsPipelineState* m_SSAOBlurPipeline;
        renderer::ShaderProgram*         m_SSAOBlurProgram;

        bool m_enableSSAO;

        //Pass 4
        RenderPass<1> m_CompositionRenderPass;
        renderer::GraphicsPipelineState* m_CompositionPipeline;
        renderer::ShaderProgram* m_CompositionProgram;

    };

} //namespace scene
} //namespace v3d
