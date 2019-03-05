#include "Scene.h"

#include "Renderer/Formats.h"
#include "Renderer/Shader.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
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

namespace v3d
{
namespace scene
{

Scene::Scene(const core::Dimension2D& size) noexcept
    : m_size(size)
    , m_camera(nullptr)
{
}

Scene::~Scene()
{
    m_camera = nullptr;

    delete m_modelDrawer;
    m_modelDrawer = nullptr;
}

void Scene::setCamera(scene::Camera * camera)
{
    m_camera = camera;
}

//void Scene::updateProgramParameters(renderer::ShaderProgram* program, scene::ModelHelper* model, scene::MaterialHelper* material)
//{
//
//   /* u32 size;
//    const void* data;
//    model->getConstantBuffer(size, data);
//    
//    program->bindUniformsBuffer<>("ubo", 0, size, data);
//
//    renderer::Texture2D* samplerColor = material->getTextureParameter<renderer::Texture2D>(scene::MaterialHeader::Property_BaseColor);
//    program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>("samplerColor", samplerColor);
//
//    renderer::Texture2D* samplerColor = material->getTextureParameter<renderer::Texture2D>(scene::MaterialHeader::Property_Specular);
//    program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>("samplerColor", samplerColor);
//
//    renderer::Texture2D* samplerColor = material->getTextureParameter<renderer::Texture2D>(scene::MaterialHeader::Property_Normals);
//    program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>("samplerColor", samplerColor);*/
//}

void Scene::onUpdate()
{
   /* struct
    {
        core::Matrix4D projection;
        core::Matrix4D model;
        core::Matrix4D view;
    } ubo;

    ubo.projection = m_camera->getProjectionMatrix();
    ubo.model = ;
    ubo.view = m_camera->getViewMatrix();

    m_modelDrawer->updateConstantBuffer(0, 0, sizeof(ubo), &ubo);*/
}

void Scene::onRender(v3d::renderer::CommandList & cmd)
{
    cmd.setViewport(core::Rect32(0, 0, m_size.width, m_size.height));
    cmd.setScissor(core::Rect32(0, 0, m_size.width, m_size.height));

    cmd.setRenderTarget(m_simpleBackbuffer);
    {
        cmd.setPipelineState(m_simplePipeline);
        {
            struct
            {
                core::Matrix4D projection;
                core::Matrix4D model;
                core::Matrix4D view;
            } ubo;

            ubo.projection = m_camera->getProjectionMatrix();
            ubo.view = m_camera->getViewMatrix();

            m_simpleProgram->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(ubo), &ubo);
        }
        m_modelDrawer->drawModel();

        //cmd.setPipelineState(m_mptSkyboxPipeline);
        //m_modelDrawer->drawModel(1);
    }
}

void Scene::onLoad(v3d::renderer::CommandList & cmd)
{
    //Load Sponza
    Model* sponza = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/4.DrawScene/data/sponza.dae");
    //m_sponzaMaterial = MaterialHelper::createMaterialHelper(cmd, sponza->);
    m_modelDrawer = ModelHelper::createModelHelper(cmd, { sponza });

    //Load skysphere
    //Model* skysphereModel = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/4.drawscene/data/skysphere.dae");
    //resource::Image* skysphereTexture = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("examples/4.drawscene/data/textures/skysphere_night.ktx");


    //Simple Draw (test)
    {
        renderer::Texture2D* depthTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);
        m_simpleBackbuffer = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_simpleBackbuffer->setColorTexture(0, cmd.getBackbuffer(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
        m_simpleBackbuffer->setDepthStencilTexture(depthTexture, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/simple.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/simple.frag");
        m_simpleProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ vertShader, fragShader }));

        m_simplePipeline = cmd.createObject<renderer::GraphicsPipelineState>(sponza->getMeshByIndex(0)->getVertexInputAttribDesc(), m_simpleProgram, m_simpleBackbuffer);
        m_simplePipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_simplePipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
        m_simplePipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_simplePipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_simplePipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_simplePipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_simplePipeline->setDepthWrite(true);
        m_simplePipeline->setDepthTest(true);
    }

    //MRT pass 1
    /*{
        renderer::Texture2D* colorTexture0 = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_R32G32B32A32_SFloat, m_size, renderer::TextureSamples::TextureSamples_x1);
        renderer::Texture2D* colorTexture1 = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_R8G8B8A8_UNorm, m_size, renderer::TextureSamples::TextureSamples_x1);
        renderer::Texture2D* colorTexture2 = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_R32G32B32A32_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);
        renderer::Texture2D* depthTexture = cmd.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_size, renderer::TextureSamples::TextureSamples_x1);

        m_offsceenRenderTarget = cmd.createObject<renderer::RenderTargetState>(m_size);
        m_offsceenRenderTarget->setColorTexture(0, colorTexture0, renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
        m_offsceenRenderTarget->setColorTexture(1, colorTexture1, renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
        m_offsceenRenderTarget->setColorTexture(2, colorTexture2, renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
        m_offsceenRenderTarget->setDepthStencilTexture(depthTexture, renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, 1.0f);

        {
            const renderer::Shader* sponzaMRTVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/mrt.vert");
            const renderer::Shader* sponzaMRTFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/mrt.frag");
            m_sponzaMRTProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ sponzaMRTVertShader , sponzaMRTFragShader }));

            m_sponzaMRTPipeline = cmd.createObject<renderer::GraphicsPipelineState>(sponza->getMeshByIndex(0)->getVertexInputAttribDesc(), m_sponzaMRTProgram, m_offsceenRenderTarget);
            m_sponzaMRTPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_sponzaMRTPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_sponzaMRTPipeline->setCullMode(renderer::CullMode::CullMode_Back);
            m_sponzaMRTPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_sponzaMRTPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
            m_sponzaMRTPipeline->setDepthWrite(true);
            m_sponzaMRTPipeline->setDepthTest(true);
        }

        {
            renderer::Shader* skysphereVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/skysphere.vert");
            renderer::Shader* skysphereFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/data/shaders/skysphere.frag");
            renderer::ShaderProgram* mrtProgram = cmd.createObject<renderer::ShaderProgram>(std::vector<const renderer::Shader*>({ skysphereVertShader , skysphereFragShader }));

            m_mptSkyboxPipeline = cmd.createObject<renderer::GraphicsPipelineState>(skysphereModel->getMeshByIndex(0)->getVertexInputAttribDesc(), mrtProgram, m_offsceenRenderTarget);
            m_mptSkyboxPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_mptSkyboxPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_mptSkyboxPipeline->setCullMode(renderer::CullMode::CullMode_Back);
            m_mptSkyboxPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_mptSkyboxPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
            m_mptSkyboxPipeline->setDepthWrite(true);
            m_mptSkyboxPipeline->setDepthTest(true);
        }
    }*/

    //SSAO pass 2
    {
        //TODO
    }

    //SSAO Blur pass 3
    {
        //TODO
    }

    cmd.sumitCommands();
    cmd.flushCommands();
}


} //namespace scene
} //namespace v3d
