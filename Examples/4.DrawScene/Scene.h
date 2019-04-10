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

        utils::IntrusivePointer<renderer::RenderTargetState>     m_SimpleBackbuffer;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_SimpleProgram;
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_SimplePipeline;

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
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTOpaquePipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTOpaqueProgram;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTTransparentPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTTransparentProgram;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTSkyboxPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTSkyboxProgram;

        utils::IntrusivePointer<renderer::VertexStreamBuffer> m_SkySphereVertexBuffer;
        utils::IntrusivePointer<renderer::IndexStreamBuffer>  m_SkySphereIndexBuffer;
        utils::IntrusivePointer<renderer::Texture2D> m_SkyTexture;


        //renderer::GraphicsPipelineState* m_MRTParticlesPipeline;

        //Pass 2
        RenderPass<1> m_SSAORenderPass;
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_SSAOPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_SSAOProgram;

        std::vector<core::Vector4D> m_SSAOKernel;
        utils::IntrusivePointer<renderer::Texture2D> m_SSAONoiseTexture;
        utils::IntrusivePointer<renderer::SamplerState> m_SSAONoiseSampler;

        //Pass 3
        RenderPass<1> m_SSAOBlurRenderPass;
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_SSAOBlurPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_SSAOBlurProgram;

        bool m_enableSSAO;

        //Pass 4
        RenderPass<1> m_CompositionRenderPass;
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_CompositionPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_CompositionProgram;
    };

} //namespace scene
} //namespace v3d
