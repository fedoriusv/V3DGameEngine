#include "SimpleRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/Shader.h"


namespace v3d
{
namespace renderer
{

SimpleRender::SimpleRender(renderer::CommandList& cmdList, const renderer::VertexInputAttribDescription& desc, const std::vector<f32>& geomentry)
{
    resource::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.vert");
    resource::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.frag");
    m_program = cmdList.createObject<ShaderProgram>(std::vector<resource::Shader*>{vertShader, fragShader});

    m_textureTarget = cmdList.createObject<Texture2D>(Format::Format_R8G8B8A8_UNorm, core::Dimension2D(1024, 768), TextureSamples::TextureSamples_x1);
    m_renderTarget = cmdList.createObject<RenderTarget>(m_textureTarget->getDimension());
    bool success = m_renderTarget->setColorTexture(0, m_textureTarget, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store);
    //m_renderTarget = cmdList.createObject<RenderTarget>(cmdList.getBackbuffer()->getDimension());
    //bool success = m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store);

    u64 vertexBufferSize = geomentry.size() * sizeof(f32);
    m_vetexBuffer = cmdList.createObject<VertexStreamBuffer>(StreamBufferUsage::StreamBuffer_Write | StreamBufferUsage::StreamBuffer_Shared, 256, (u8*)geomentry.data());

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(desc, m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);

    cmdList.setPipelineState(m_pipeline);
    cmdList.setRenderTarget(m_renderTarget);

    core::Matrix4D projection;
    renderer::Texture2D* texture;

    cmdList.sumitCommands();
    cmdList.flushCommands();
}

SimpleRender::~SimpleRender()
{
    delete m_pipeline;
    delete m_program;
    delete m_renderTarget;
    delete m_textureTarget;

    //delete shaders
}

void SimpleRender::update(f32 zoom, const core::Vector3D& rotate)
{
    struct
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } uboVS;

    uboVS.projectionMatrix = core::buildProjectionMatrixPerspective(60.0f, f32(m_renderTarget->getDimension().width / m_renderTarget->getDimension().height), 0.1f, 256.0f);
    uboVS.modelMatrix.makeIdentity();
    uboVS.viewMatrix.setTranslation(core::Vector3D(0.0f, 0.0f, zoom));
    uboVS.modelMatrix.makeIdentity();
    uboVS.modelMatrix.setRotation(rotate);

    m_program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(uboVS), &uboVS);
}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));

    SimpleRender::update(1.0f, core::Vector3D(0.0f));
    cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0), 0, 3, 1);

    SimpleRender::update(2.0f, core::Vector3D(0.0f, 90.0f, 0.0f));
    cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0), 0, 3, 1);
}

} //namespace renderer
} //namespace v3d
