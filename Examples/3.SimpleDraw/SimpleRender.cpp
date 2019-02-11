#include "SimpleRender.h"

#include "Resource/ResourceLoaderManager.h"

#include "Resource/ShaderSourceFileLoader.h"
#include "Renderer/Shader.h"

#include "Resource/ModelFileLoader.h"
#include "Scene/Model.h"

#include "Scene/Camera.h"


namespace v3d
{
namespace renderer
{

SimpleRender::SimpleRender(renderer::CommandList& cmdList, const renderer::VertexInputAttribDescription& desc, const std::vector<f32>& geomentry)
{
    Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.vert");
    Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.frag");
    m_program = cmdList.createObject<ShaderProgram>(std::vector<Shader*>{vertShader, fragShader});

    scene::Model* mesh = resource::ResourceLoaderManager::getInstance()->loadMesh<scene::Model, resource::ModelFileLoader>(cmdList.getContext(), "examples/3.simpledraw/models/voyager/voyager.dae");

    //Texture2D* colorAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_R8G8B8A8_UNorm, core::Dimension2D(1024, 768), TextureSamples::TextureSamples_x1);
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt/*Format_D24_UNorm_S8_UInt*/, core::Dimension2D(1024, 768), TextureSamples::TextureSamples_x1);

    m_renderTarget = cmdList.createObject<RenderTarget>(cmdList.getBackbuffer()->getDimension());
    //m_renderTarget->setColorTexture(0, colorAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    u64 vertexBufferSize = geomentry.size() * sizeof(f32);
    m_vetexBuffer = cmdList.createObject<VertexStreamBuffer>(StreamBufferUsage::StreamBuffer_Write | StreamBufferUsage::StreamBuffer_Shared, vertexBufferSize, (u8*)geomentry.data());

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(desc, m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(CullMode::CullMode_None);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);

    cmdList.setPipelineState(m_pipeline);
    cmdList.setRenderTarget(m_renderTarget);
    cmdList.setRenderTarget(m_renderTarget);

    core::Matrix4D projection;
    renderer::Texture2D* texture;

    cmdList.sumitCommands();
    cmdList.flushCommands();

    m_camera = new scene::CameraHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 1.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, 0.0f));
    m_camera->getCamera().setFOV(45.0f);
    m_camera->getCamera().setNearValue(0.1f);
    m_camera->getCamera().setFarValue(20.0f);
}

SimpleRender::~SimpleRender()
{
    delete m_pipeline;
    delete m_program;
    delete m_renderTarget;

    delete m_camera;
    
    //TODO:
    //delete m_textureTarget;
    //delete shaders
}

void SimpleRender::update(const core::Vector3D& pos, const core::Vector3D& rotate)
{
    m_camera->update(m_renderTarget->getDimension());

    struct
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } uboVS, uboVS1;

    uboVS.projectionMatrix = m_camera->getCamera().getProjectionMatrix();
    uboVS.viewMatrix = m_camera->getCamera().getViewMatrix();

    uboVS.modelMatrix.makeIdentity();
    uboVS.modelMatrix.setRotation(rotate);
    uboVS.modelMatrix.setTranslation(pos);
    //uboVS.modelMatrix.makeTransposed();

    m_program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(uboVS), &uboVS);
}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));

    SimpleRender::update(core::Vector3D(0.0f, 0.0f, 5.1f), core::Vector3D(0.0f, 0.0f, 40.0f));
    cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0), 0, 3, 1);
    SimpleRender::update(core::Vector3D(0.0f, 0.0f, 3.0f), core::Vector3D(0.0f));
    cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0), 0, 3, 1);
}

} //namespace renderer
} //namespace v3d
