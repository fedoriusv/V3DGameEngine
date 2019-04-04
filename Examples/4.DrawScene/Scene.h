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

    private:

        renderer::RenderTargetState* m_simpleBackbuffer;
        renderer::ShaderProgram* m_simpleProgram;
        renderer::GraphicsPipelineState* m_simplePipeline;

        //void updateProgramParameters(renderer::ShaderProgram* program, scene::ModelHelper* model, scene::MaterialHelper* material);

        renderer::GraphicsPipelineState* m_mptParticlesPipeline;
        renderer::GraphicsPipelineState* m_mptSkyboxPipeline;


        renderer::GraphicsPipelineState* m_sponzaPipeline;
        renderer::ShaderProgram* m_sponzaProgram;

        std::vector<scene::MaterialHelper*> m_sponzaMaterials;
        scene::ModelHelper* m_modelDrawer;




        utils::IntrusivePointer<renderer::SamplerState> m_Sampler;

        core::Dimension2D m_size;
        scene::Camera*    m_camera;

        template<u32 attachments>
        struct RenderPass
        {
            std::array<renderer::Texture2D*, attachments> colorTexture;
            renderer::Texture2D*                          depthTexture;

            renderer::RenderTargetState*                  renderTarget;
        };


        RenderPass<3> m_MRTRenderPass;


        RenderPass<1> m_SSAORenderPass;
        renderer::GraphicsPipelineState* m_SSAOPipeline;
        renderer::ShaderProgram* m_SSAOProgram;

        std::vector<core::Vector4D> m_SSAOKernel;
        utils::IntrusivePointer<renderer::Texture2D> m_SSAONoiseTexture;
        utils::IntrusivePointer<renderer::SamplerState> m_SSAONoiseSampler;

        RenderPass<1> m_CompositionRenderPass;
        renderer::GraphicsPipelineState* m_CompositionPipeline;
        renderer::ShaderProgram* m_CompositionProgram;
        utils::IntrusivePointer<renderer::VertexStreamBuffer> m_ScreenQuad;

        utils::IntrusivePointer<renderer::Texture2D> m_DummyTexture;
    };

} //namespace scene
} //namespace v3d
