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
    class ParticleSystemHelper;

    class Scene
    {
    public:
        Scene(const core::Dimension2D& size) noexcept;
        ~Scene();

        void onLoad(v3d::renderer::CommandList & cmd);

        void onUpdate(f32 dt);
        void onRender(v3d::renderer::CommandList& cmd);

        void setCamera(scene::Camera* camera);
        void setCameraPosition(const core::Vector3D& position);

    private:

        utils::IntrusivePointer<renderer::RenderTargetState>     m_SimpleBackbuffer;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_SimpleProgram;
        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_SimplePipeline;

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
        std::vector<scene::MaterialHelper*> m_SponzaMaterials;
        scene::ModelHelper* m_SponzaModelDrawer;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTOpaquePipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTOpaqueProgram;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTTransparentPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTTransparentProgram;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTSkyboxPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTSkyboxProgram;

        utils::IntrusivePointer<renderer::VertexStreamBuffer> m_SkySphereVertexBuffer;
        utils::IntrusivePointer<renderer::IndexStreamBuffer>  m_SkySphereIndexBuffer;
        utils::IntrusivePointer<renderer::Texture2D> m_SkyTexture;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_MRTParticlesPipeline;
        utils::IntrusivePointer<renderer::ShaderProgram>         m_MRTParticlesProgram;
        utils::IntrusivePointer<renderer::Texture2D> m_ParticleFireTexture;
        utils::IntrusivePointer<renderer::SamplerState> m_ParticleFireSampler;
        utils::IntrusivePointer<renderer::Texture2D> m_ParticleSmokeTexture;
        utils::IntrusivePointer<renderer::SamplerState> m_ParticleSmokeSampler;
        ParticleSystemHelper* m_ParticleSystem;

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


        struct Light
        {
            core::Vector4D  _position;
            core::Vector4D  _color;
            f32             _radius;
            f32             _pad[3];
        };

        std::vector<Light> m_lights;

        void setupLights();


    };

} //namespace scene
} //namespace v3d
