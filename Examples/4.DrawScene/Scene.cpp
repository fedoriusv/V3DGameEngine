#include "Scene.h"

#include "Renderer/Formats.h"
#include "Renderer/Shader.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/SamplerState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/StreamBuffer.h"

#include "Resource/Image.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

#include "Scene/Model.h"
#include "Scene/ModelHelper.h"
#include "Scene/Material.h"
#include "Scene/MaterialHelper.h"
#include "Scene/Camera.h"
#include "Scene/ParticleSystem.h"
#include "Scene/ParticleSystemHelper.h"

#define TEST_DRAW 0

#define SSAO_KERNEL_SIZE 32
#define SSAO_RADIUS 2.0f
#define SSAO_NOISE_DIM 4

#define LIGHTS_COUNT 17

namespace v3d
{
namespace scene
{

Scene::Scene(const core::Dimension2D& size) noexcept
    : m_size(size)
    , m_camera(nullptr)
    , m_enableSSAO(true)
{
    resource::ResourceLoaderManager::getInstance()->addPath("../../../../examples/4.DrawScene/data/");
}

Scene::~Scene()
{
    m_camera = nullptr;

    for (auto material : m_SponzaMaterials)
    {
        delete material;
    }
    m_SponzaMaterials.clear();

    delete m_SponzaModelDrawer;
    m_SponzaModelDrawer = nullptr;

    if (m_ParticleSystem)
    {
        delete m_ParticleSystem;
        m_ParticleSystem = nullptr;
    }

    m_Sampler = nullptr;
    m_DummyTexture = nullptr;
}

void Scene::setCamera(scene::Camera * camera)
{
    m_camera = camera;
}

void Scene::setCameraPosition(const core::Vector3D & position)
{
    m_viewPosition = position;
}

void Scene::setupLights()
{
    m_lights.resize(LIGHTS_COUNT);

    auto setupLight = [](Light *light, const core::Vector4D& pos, const core::Vector4D& color, f32 radius) -> void
    {
        light->_position = pos;
        light->_color = color;
        light->_radius = radius;
    };


    std::array<core::Vector4D, 5> lightColors;
    lightColors[0] = core::Vector4D(1.0f, 0.0f, 0.0f, 1.0f);
    lightColors[1] = core::Vector4D(1.0f, 0.7f, 0.7f, 1.0f);
    lightColors[2] = core::Vector4D(1.0f, 0.0f, 0.0f, 1.0f);
    lightColors[3] = core::Vector4D(0.0f, 0.0f, 1.0f, 1.0f);
    lightColors[4] = core::Vector4D(1.0f, 0.0f, 0.0f, 1.0f);

    for (u32 i = 0; i < lightColors.size(); i++)
    {
        setupLight(&m_lights[i], core::Vector4D((f32)(i - 2.5f) * 50.0f, 10.0f, 0.0f, 1.0f), lightColors[i], 120.0f);
    }

    // Dynamic light moving over the floor
    setupLight(&m_lights[0], { 0.0f * 120.0f, 2.5f, 1.0f * 10.0f }, core::Vector4D(1.0f), 100.0f);

    // Fire bowls
    setupLight(&m_lights[5], { -48.75f, 16.0f, -17.8f, 1.0f }, { 1.0f, 0.6f, 0.0f, 1.0f }, 45.0f);
    setupLight(&m_lights[6], { -48.75f, 16.0f,  18.4f, 1.0f }, { 1.0f, 0.6f, 0.0f, 1.0f }, 45.0f);
    setupLight(&m_lights[7], { 62.0f, 16.0f, -17.8f, 1.0f },   { 1.0f, 0.6f, 0.0f, 1.0f }, 45.0f);
    setupLight(&m_lights[8], { 62.0f, 16.0f,  18.4f, 1.0f },   { 1.0f, 0.6f, 0.0f, 1.0f }, 45.0f);

    setupLight(&m_lights[9], { 120.0f, 20.0f, -43.75f, 1.0f },     { 1.0f, 0.8f, 0.3f, 1.0f }, 75.0f);
    setupLight(&m_lights[10], { 120.0f, 20.0f, 41.75f, 1.0f },     { 1.0f, 0.8f, 0.3f, 1.0f }, 75.0f);
    setupLight(&m_lights[11], { -110.0f, 20.0f, -43.75f, 1.0f },   { 1.0f, 0.8f, 0.3f, 1.0f }, 75.0f);
    setupLight(&m_lights[12], { -110.0f, 20.0f, 41.75f, 1.0f },    { 1.0f, 0.8f, 0.3f, 1.0f }, 75.0f);

    // Lion eyes
    setupLight(&m_lights[13], { -122.0f, 18.0f, -3.2f, 1.0f }, { 1.0f, 0.3f, 0.3f, 1.0f }, 25.0f);
    setupLight(&m_lights[14], { -122.0f, 18.0f,  3.2f, 1.0f }, { 0.3f, 1.0f, 0.3f, 1.0f }, 25.0f);

    setupLight(&m_lights[15], { 135.0f, 18.0f, -3.2f, 1.0f }, { 0.3f, 0.3f, 1.0f, 1.0f }, 25.0f);
    setupLight(&m_lights[16], { 135.0f, 18.0f,  3.2f, 1.0f }, { 1.0f, 1.0f, 0.3f, 1.0f }, 25.0f);

    // Setup particle systems for fire bowls
    for (u32 i = 5; i < 9; i++)
    {
        m_ParticleSystem->add(512, core::Vector3D(m_lights[i]._position.x, m_lights[i]._position.y, m_lights[i]._position.z) + core::Vector3D(0.0f, 2.5f, 0.0f),
            core::Vector3D(-2.0f, 0.25f, -2.0f), core::Vector3D(2.0f, 2.5f, 2.0f));
    }
}

void Scene::onUpdate(f32 dt)
{
    const f32 timerSpeed = 0.25f;
    static f32 timer;
    timer += timerSpeed * dt;
    if (timer > 1.0)
    {
        timer -= 1.0f;
    }

    // Move across the floow
    m_lights[0]._position.x = -sin(core::k_degToRad * 360.0f * timer) * 120.0f;
    m_lights[0]._position.z = cos(core::k_degToRad * 360.0f * timer * 8.0f) * 10.0f;

    m_ParticleSystem->update(dt);
}

void Scene::onRender(v3d::renderer::CommandList & cmd)
{
#if TEST_DRAW
    cmd.setViewport(core::Rect32(0, 0, m_size.width, m_size.height));
    cmd.setScissor(core::Rect32(0, 0, m_size.width, m_size.height));

    cmd.setRenderTarget(m_SimpleBackbuffer.get());

    {
        cmd.setPipelineState(m_SimplePipeline.get());
        {
            struct
            {
                core::Matrix4D projection;
                core::Matrix4D model;
                core::Matrix4D view;
            } ubo;

            ubo.projection = m_camera->getProjectionMatrix();
            ubo.view = m_camera->getViewMatrix();

            m_SimpleProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(ubo), &ubo);
        }
        m_modelDrawer->draw(0);

        //cmd.setPipelineState(m_mptSkyboxPipeline);
        //m_modelDrawer->drawModel(1);
    }
#else
    //MRT
    {
        cmd.setViewport(core::Rect32(0, 0, m_size.width, m_size.height));
        cmd.setScissor(core::Rect32(0, 0, m_size.width, m_size.height));

        cmd.setRenderTarget(m_MRTRenderPass.renderTarget.get());

        struct
        {
            core::Matrix4D projection;
            core::Matrix4D view;
            core::Matrix4D model;
        } ubo;

        ubo.projection = m_camera->getProjectionMatrix();
        ubo.view = m_camera->getViewMatrix();
        ubo.model.makeIdentity();

        cmd.setPipelineState(m_MRTOpaquePipeline.get());
        for (u32 i = 0; i < m_SponzaModelDrawer->getDrawStatesCount(); ++i)
        {
            if (m_SponzaMaterials[i]->getFloatParameter(MaterialHeader::Property::Property_Opacity) > 0.0f)
            {
                continue;
            }

            renderer::Texture2D* baseColor = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_BaseColor);
            renderer::Texture2D* specularColor = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_Specular);
            renderer::Texture2D* normalMap = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_Normals);

            m_MRTOpaqueProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>(0/*"ubo"*/, 0, sizeof(ubo), &ubo);
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerColor"*/, baseColor ? baseColor : m_DummyTexture.get(), m_Sampler.get());
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(1/*"samplerSpecular"*/, specularColor ? specularColor : m_DummyTexture.get(), m_Sampler.get());
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(2/*"samplerNormal"*/, normalMap ? normalMap : m_DummyTexture.get(), m_Sampler.get());

            m_SponzaModelDrawer->draw(i);
        }

        {
            cmd.setPipelineState(m_MRTSkyboxPipeline.get());

            struct
            {
                core::Matrix4D projection;
                core::Matrix4D view;
                core::Matrix4D model;
                core::Vector3D viewPos;
            } ubo;

            ubo.projection = m_camera->getProjectionMatrix();
            ubo.view = m_camera->getViewMatrix();
            ubo.model.makeIdentity();
            ubo.model.setScale(core::Vector3D(80.0f));
            ubo.viewPos = m_viewPosition;

            m_MRTSkyboxProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>(0/*"ubo"*/, 0, sizeof(ubo), &ubo);
            m_MRTSkyboxProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerSky"*/, m_SkyTexture.get(), m_Sampler.get());

            cmd.drawIndexed(renderer::StreamBufferDescription(m_SkySphereIndexBuffer.get(), 0, m_SkySphereVertexBuffer.get(), 0, 0), 0, m_SkySphereIndexBuffer->getIndexCount(), 1);
        }

        cmd.setPipelineState(m_MRTTransparentPipeline.get());
        for (u32 i = 0; i < m_SponzaModelDrawer->getDrawStatesCount(); ++i)
        {
            if (m_SponzaMaterials[i]->getFloatParameter(MaterialHeader::Property::Property_Opacity) == 0.0f)
            {
                continue;
            }

            renderer::Texture2D* baseColor = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_BaseColor);
            renderer::Texture2D* specularColor = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_Specular);
            renderer::Texture2D* normalMap = m_SponzaMaterials[i]->getTextureParameter<renderer::Texture2D>(MaterialHeader::Property::Property_Normals);

            m_MRTOpaqueProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>(0/*"ubo"*/, 0, sizeof(ubo), &ubo);
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerColor"*/, baseColor ? baseColor : m_DummyTexture.get(), m_Sampler.get());
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(1/*"samplerSpecular"*/, specularColor ? specularColor : m_DummyTexture.get(), m_Sampler.get());
            m_MRTOpaqueProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(2/*"samplerNormal"*/, normalMap ? normalMap : m_DummyTexture.get(), m_Sampler.get());

            m_SponzaModelDrawer->draw(i);
        }
    }

    //SSAO
    {
        cmd.setRenderTarget(m_SSAORenderPass.renderTarget.get());
        cmd.setPipelineState(m_SSAOPipeline.get());

        m_SSAOProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>(0/*"uboSSAOKernel"*/, 0, sizeof(core::Vector4D) * static_cast<u32>(m_SSAOKernel.size()), m_SSAOKernel.data());
        m_SSAOProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>(1/*"ubo"*/, 0, sizeof(core::Matrix4D), m_camera->getProjectionMatrix().getPtr());
        m_SSAOProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerPositionDepth"*/, m_MRTRenderPass.colorTexture[0].get(), m_Sampler.get());
        m_SSAOProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(1/*"samplerNormal"*/, m_MRTRenderPass.colorTexture[1].get(), m_Sampler.get());
        m_SSAOProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(2/*"ssaoNoise"*/, m_SSAONoiseTexture.get(), m_Sampler.get());

        cmd.draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
    }

    // SSAO Blur
    {
        cmd.setRenderTarget(m_SSAOBlurRenderPass.renderTarget.get());
        cmd.setPipelineState(m_SSAOBlurPipeline.get());

        m_SSAOBlurProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerSSAO"*/, m_SSAORenderPass.colorTexture[0].get(), m_Sampler.get());

        cmd.draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
    }

    cmd.setViewport(core::Rect32(0, 0, cmd.getBackbuffer()->getDimension().width, cmd.getBackbuffer()->getDimension().height));
    cmd.setScissor(core::Rect32(0, 0, cmd.getBackbuffer()->getDimension().width, cmd.getBackbuffer()->getDimension().height));

    //Composition
    {
        cmd.setRenderTarget(m_CompositionRenderPass.renderTarget.get());

        {
            cmd.setPipelineState(m_CompositionPipeline.get());

            struct
            {
                core::Matrix4D view;
                core::Matrix4D model;
                core::Vector4D viewPos;
                Light light[LIGHTS_COUNT];
            } ubo;

            ubo.view = m_camera->getViewMatrix();
            ubo.model.makeIdentity();
            ubo.viewPos = core::Vector4D(m_viewPosition, 0.0f);
            memcpy(&ubo.light, m_lights.data(), sizeof(Light) * LIGHTS_COUNT);

            m_CompositionProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>(0/*"ubo"*/, 0, sizeof(ubo), &ubo);
            m_CompositionProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerPosition"*/, m_MRTRenderPass.colorTexture[0].get(), m_Sampler.get());
            m_CompositionProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(1/*"samplerNormal"*/, m_MRTRenderPass.colorTexture[1].get(), m_Sampler.get());
            m_CompositionProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(2/*"samplerAlbedo"*/, m_MRTRenderPass.colorTexture[2].get(), m_Sampler.get());
            m_CompositionProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(3/*"samplerSSAO"*/, m_SSAOBlurRenderPass.colorTexture[0].get(), m_Sampler.get());

            cmd.draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
        }

        {
            cmd.setPipelineState(m_MRTParticlesPipeline.get());

            struct
            {
                core::Matrix4D projection;
                core::Matrix4D view;
                core::Matrix4D model;
                core::Vector2D viewportDim;
            } ubo;

            ubo.projection = m_camera->getProjectionMatrix();
            ubo.view = m_camera->getViewMatrix();
            ubo.model.makeIdentity();
            ubo.viewportDim = core::Vector2D(f32(cmd.getBackbuffer()->getDimension().width), f32(cmd.getBackbuffer()->getDimension().height));

            m_MRTParticlesProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>(0/*"ubo"*/, 0, sizeof(ubo), &ubo);
            m_MRTParticlesProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(0/*"samplerSmoke"*/, m_ParticleSmokeTexture.get(), m_Sampler.get());
            m_MRTParticlesProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(1/*"samplerFire"*/, m_ParticleFireTexture.get(), m_Sampler.get());
            m_MRTParticlesProgram->bindSampledTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>(2/*"samplerPositionDepth"*/, m_MRTRenderPass.colorTexture[0].get(), m_Sampler.get());

            m_ParticleSystem->draw();
        }
    }
#endif
}

void Scene::onLoad(v3d::renderer::CommandList & cmd)
{
    m_Sampler = cmd.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_8x);

    resource::Image* dummyImage = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("sponza/dummy.dds");
    m_DummyTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, dummyImage->getFormat(), core::Dimension2D(dummyImage->getDimension().width, dummyImage->getDimension().height), 1, 1, dummyImage->getRawData());
    
    //Load Sponza
    Model* sponza = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("sponza.dae", 
        resource::ModelLoaderFlag::ModelLoaderFlag_SeperateMesh | resource::ModelLoaderFlag::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
    m_SponzaMaterials = MaterialHelper::createMaterialHelpers(cmd, sponza->getMaterials());
    m_SponzaModelDrawer = ModelHelper::createModelHelper(cmd, { sponza });

#if TEST_DRAW
    //Simple Draw (test)
    {
        renderer::Texture2D* depthTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);
        m_SimpleBackbuffer = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_SimpleBackbuffer->setColorTexture(0, cmd.getBackbuffer(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
        m_SimpleBackbuffer->setDepthStencilTexture(depthTexture, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/simple.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/simple.frag");
        m_SimpleProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ vertShader, fragShader }));

        m_SimplePipeline = cmd.createObject<renderer::GraphicsPipelineState>(sponza->getMeshByIndex(0)->getVertexInputAttribDesc(), m_SimpleProgram.get(), m_SimpleBackbuffer.get());
        m_SimplePipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_SimplePipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
        m_SimplePipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_SimplePipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_SimplePipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_SimplePipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_SimplePipeline->setDepthWrite(true);
        m_SimplePipeline->setDepthTest(true);
    }
#else
    //MRT pass 1
    {
        m_MRTRenderPass.colorTexture[0] = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R32G32B32A32_SFloat, m_size, renderer::TextureSamples::TextureSamples_x1);
        m_MRTRenderPass.colorTexture[1] = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8G8B8A8_UNorm, m_size, renderer::TextureSamples::TextureSamples_x1);
        m_MRTRenderPass.colorTexture[2] = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R32G32B32A32_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);
        m_MRTRenderPass.depthTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);

        renderer::RenderTargetState::ColorOpState colorOpState = { renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) };
        renderer::RenderTargetState::TransitionState tansitionState = { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead };
        m_MRTRenderPass.renderTarget = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_MRTRenderPass.renderTarget->setColorTexture(0, m_MRTRenderPass.colorTexture[0].get(), colorOpState, tansitionState);
        m_MRTRenderPass.renderTarget->setColorTexture(1, m_MRTRenderPass.colorTexture[1].get(), colorOpState, tansitionState);
        m_MRTRenderPass.renderTarget->setColorTexture(2, m_MRTRenderPass.colorTexture[2].get(), colorOpState, tansitionState);
        m_MRTRenderPass.renderTarget->setDepthStencilTexture(m_MRTRenderPass.depthTexture.get(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

        {
            std::vector<std::pair<std::string, std::string>> defines =
            {
                { "NEAR_PLANE", std::to_string(m_camera->getNearValue()) },
                { "FAR_PLANE", std::to_string(m_camera->getFarValue()) },
                { "ENABLE_DISCARD", std::to_string(0) }
            };

            const renderer::Shader* sponzaMRTVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/mrt.vert");
            const renderer::Shader* sponzaMRTFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/mrt.frag", defines);
            m_MRTOpaqueProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ sponzaMRTVertShader , sponzaMRTFragShader }));

            m_MRTOpaquePipeline = cmd.createObject<renderer::GraphicsPipelineState>(sponza->getMeshByIndex(0)->getVertexInputAttribDesc(), m_MRTOpaqueProgram.get(), m_MRTRenderPass.renderTarget.get());
            m_MRTOpaquePipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_MRTOpaquePipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_MRTOpaquePipeline->setCullMode(renderer::CullMode::CullMode_Back);
            m_MRTOpaquePipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_MRTOpaquePipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
            m_MRTOpaquePipeline->setDepthWrite(true);
            m_MRTOpaquePipeline->setDepthTest(true);
        }

        {
            std::vector<std::pair<std::string, std::string>> defines =
            {
                { "NEAR_PLANE", std::to_string(m_camera->getNearValue()) },
                { "FAR_PLANE", std::to_string(m_camera->getFarValue()) },
                { "ENABLE_DISCARD", std::to_string(1) }
            };

            const renderer::Shader* sponzaMRTVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/mrt.vert");
            const renderer::Shader* sponzaMRTFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/mrt.frag", defines);
            m_MRTTransparentProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ sponzaMRTVertShader , sponzaMRTFragShader }));

            m_MRTTransparentPipeline = cmd.createObject<renderer::GraphicsPipelineState>(sponza->getMeshByIndex(0)->getVertexInputAttribDesc(), m_MRTTransparentProgram.get(), m_MRTRenderPass.renderTarget.get());
            m_MRTTransparentPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_MRTTransparentPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_MRTTransparentPipeline->setCullMode(renderer::CullMode::CullMode_Back);
            m_MRTTransparentPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_MRTTransparentPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
            m_MRTTransparentPipeline->setDepthWrite(false);
            m_MRTTransparentPipeline->setDepthTest(true);
        }

        {
            //Load skysphere
            Model* skysphereModel = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/4.drawscene/data/skysphere.dae", resource::ModelLoaderFlag::ModelLoaderFlag_SeperateMesh);
            resource::Image* skysphereImage = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("examples/4.drawscene/data/textures/skysphere_night.ktx");
            m_SkyTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, skysphereImage->getFormat(), core::Dimension2D(skysphereImage->getDimension().width, skysphereImage->getDimension().height), 1, 1, skysphereImage->getRawData());
            m_SkySphereVertexBuffer = cmd.createObject<renderer::VertexStreamBuffer>(renderer::StreamBufferUsage::StreamBuffer_Write, skysphereModel->getMeshByIndex(0)->getVertexSize(), skysphereModel->getMeshByIndex(0)->getVertexData());
            m_SkySphereIndexBuffer = cmd.createObject<renderer::IndexStreamBuffer>(renderer::StreamBufferUsage::StreamBuffer_Write, renderer::StreamIndexBufferType::IndexType_32, skysphereModel->getMeshByIndex(0)->getIndexCount(), skysphereModel->getMeshByIndex(0)->getIndexData());

            renderer::Shader* skysphereVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/skysphere.vert");
            renderer::Shader* skysphereFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/skysphere.frag");
            m_MRTSkyboxProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ skysphereVertShader , skysphereFragShader }));

            m_MRTSkyboxPipeline = cmd.createObject<renderer::GraphicsPipelineState>(skysphereModel->getMeshByIndex(0)->getVertexInputAttribDesc(), m_MRTSkyboxProgram.get(), m_MRTRenderPass.renderTarget.get());
            //m_MRTSkyboxPipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
            m_MRTSkyboxPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_MRTSkyboxPipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
            m_MRTSkyboxPipeline->setCullMode(renderer::CullMode::CullMode_None);
            m_MRTSkyboxPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_MRTSkyboxPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
            m_MRTSkyboxPipeline->setDepthWrite(true);
            m_MRTSkyboxPipeline->setDepthTest(true);
        }
    }

    //SSAO pass 2
    {
        {
            std::uniform_real_distribution<f32> rndDist(0.0f, 1.0f);
            std::random_device rndDev;
            std::default_random_engine rndGen;

            auto lerp = [](f32 a, f32 b, f32 f) -> f32
            {
                return a + f * (b - a);
            };

            // Sample kernel
            m_SSAOKernel.resize(SSAO_KERNEL_SIZE);
            for (u32 i = 0; i < SSAO_KERNEL_SIZE; ++i)
            {
                core::Vector3D sample(rndDist(rndGen) * 2.0f - 1.0f, rndDist(rndGen) * 2.0f - 1.0f, rndDist(rndGen));
                sample.normalize();
                sample *= rndDist(rndGen);
                f32 scale = f32(i) / f32(SSAO_KERNEL_SIZE);
                scale = lerp(0.1f, 1.0f, scale * scale);
                m_SSAOKernel[i] = core::Vector4D(sample * scale, 0.0f);
            }

            // Random noise
            std::vector<core::Vector4D> ssaoNoise(SSAO_NOISE_DIM * SSAO_NOISE_DIM);
            for (u32 i = 0; i < ssaoNoise.size(); i++)
            {
                ssaoNoise[i] = core::Vector4D(rndDist(rndGen) * 2.0f - 1.0f, rndDist(rndGen) * 2.0f - 1.0f, 0.0f, 0.0f);
            }
            m_SSAONoiseTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write, renderer::Format::Format_R32G32B32A32_SFloat, core::Dimension2D(SSAO_NOISE_DIM, SSAO_NOISE_DIM), 1, 1, ssaoNoise.data());
        }

        std::vector<std::pair<std::string, std::string>> defines = 
        {
            { "SSAO_KERNEL_SIZE", std::to_string(SSAO_KERNEL_SIZE) },
            { "SSAO_RADIUS", std::to_string(SSAO_RADIUS) },
            { "SSAO_POWER", std::to_string(1.5f) }
        };

        const renderer::Shader* fullscreenVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/fullscreen.vert");
        const renderer::Shader* ssaoFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/ssao.frag", defines);
        m_SSAOProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ fullscreenVertShader , ssaoFragShader }));

        m_SSAORenderPass.colorTexture[0] = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8_UNorm, m_size, renderer::TextureSamples::TextureSamples_x1);

        renderer::RenderTargetState::ColorOpState colorOpState = { renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) };
        renderer::RenderTargetState::TransitionState tansitionState = { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead };
        m_SSAORenderPass.renderTarget = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_SSAORenderPass.renderTarget->setColorTexture(0, m_SSAORenderPass.colorTexture[0].get(), colorOpState, tansitionState);

        m_SSAOPipeline = cmd.createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_SSAOProgram.get(), m_SSAORenderPass.renderTarget.get());
        m_SSAOPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    }

    //SSAO Blur pass 3
    {
        const renderer::Shader* fullscreenVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/fullscreen.vert");
        const renderer::Shader* ssaoBlurFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/blur.frag");
        m_SSAOBlurProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ fullscreenVertShader , ssaoBlurFragShader }));

        m_SSAOBlurRenderPass.colorTexture[0] = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8_UNorm, m_size, renderer::TextureSamples::TextureSamples_x1);

        renderer::RenderTargetState::ColorOpState colorOpState = { renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) };
        renderer::RenderTargetState::TransitionState tansitionState = { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead };
        m_SSAOBlurRenderPass.renderTarget = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_SSAOBlurRenderPass.renderTarget->setColorTexture(0, m_SSAOBlurRenderPass.colorTexture[0].get(), colorOpState, tansitionState);

        m_SSAOBlurPipeline = cmd.createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_SSAOBlurProgram.get(), m_SSAOBlurRenderPass.renderTarget.get());
        m_SSAOBlurPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    }

    //Composition pass 4
    {
        {
            std::vector<std::pair<std::string, std::string>> defines =
            {
                { "SSAO_ENABLED", std::to_string(m_enableSSAO) },
                { "AMBIENT_FACTOR", std::to_string(0.15f) },
                { "NUM_LIGHTS", std::to_string(LIGHTS_COUNT) },
            };

            renderer::RenderTargetState::ColorOpState colorOpState = { renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) };
            renderer::RenderTargetState::TransitionState tansitionState = { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present };
            m_CompositionRenderPass.renderTarget = cmd.createObject<renderer::RenderTargetState>(m_size);
            m_CompositionRenderPass.renderTarget->setColorTexture(0, cmd.getBackbuffer(), colorOpState, tansitionState);

            const renderer::Shader* fullscreenVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/fullscreen.vert");
            const renderer::Shader* compositionFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/composition.frag", defines);
            m_CompositionProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ fullscreenVertShader , compositionFragShader }));

            m_CompositionPipeline = cmd.createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription()/*screenQuadDesc*/, m_CompositionProgram.get(), m_CompositionRenderPass.renderTarget.get());
            m_CompositionPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        }

        {
            std::vector<std::pair<std::string, std::string>> defines =
            {
                { "NEAR_PLANE", std::to_string(m_camera->getNearValue()) },
                { "FAR_PLANE", std::to_string(m_camera->getFarValue()) },
            };

            resource::Image* particleFireImage = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("textures/particle_fire.ktx");
            m_ParticleFireTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, particleFireImage->getFormat(), core::Dimension2D(particleFireImage->getDimension().width, particleFireImage->getDimension().height),
                particleFireImage->getLayersCount(), 1/*particleFireImage->getMipMapsCount()*/, particleFireImage->getRawData());

            resource::Image* particleSmokeImage = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("textures/particle_smoke.ktx");
            m_ParticleSmokeTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, particleSmokeImage->getFormat(), core::Dimension2D(particleSmokeImage->getDimension().width, particleSmokeImage->getDimension().height),
                particleSmokeImage->getLayersCount(), 1/*particleSmokeImage->getMipMapsCount()*/, particleSmokeImage->getRawData());

            m_ParticleSystem = ParticleSystemHelper::createParticleSystemHelper(cmd);

            renderer::Shader* particleVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/particle.vert");
            renderer::Shader* particleFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "shaders/particle.frag", defines);
            m_MRTParticlesProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ particleVertShader , particleFragShader }));

            m_MRTParticlesPipeline = cmd.createObject<renderer::GraphicsPipelineState>(m_ParticleSystem->getVertexInputAttribDesc(), m_MRTParticlesProgram.get(), m_CompositionRenderPass.renderTarget.get());
            m_MRTParticlesPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_PointList);
            m_MRTParticlesPipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
            m_MRTParticlesPipeline->setCullMode(renderer::CullMode::CullMode_None);
            m_MRTParticlesPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_MRTParticlesPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
            //blend
        }

        setupLights();
    }

#endif

    cmd.sumitCommands();
    cmd.flushCommands();
}


} //namespace scene
} //namespace v3d
