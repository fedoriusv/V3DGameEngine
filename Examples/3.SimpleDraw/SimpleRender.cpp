#include "SimpleRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"


namespace v3d
{
SimpleRender::SimpleRender(renderer::CommandList& cmdList, const std::vector<f32>& geomentry)
{
    resource::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.vert");
    resource::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.frag");
    m_program = cmdList.createObject<renderer::ShaderProgram>(std::vector<resource::Shader*>{vertShader, fragShader});

    m_texture = cmdList.createObject<renderer::Texture2D>(renderer::Format::Format_R8G8B8A8_UInt, core::Dimension2D(1024, 768), renderer::TextureSamples::TextureSamples_x1);
    m_renderTarget = cmdList.createObject<renderer::RenderTarget>(m_texture->getDimension());
    bool success = m_renderTarget->setColorTexture(0, m_texture, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store);

    m_pipeline = cmdList.createObject<renderer::GraphicsPipelineState>(m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);

    u64 vertexBufferSize = geomentry.size() * sizeof(f32);
    m_vetexBuffer = cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, vertexBufferSize, geomentry.data());

    cmdList.setPipelineState(m_pipeline);
    cmdList.setRenderTarget(m_renderTarget);


    //core::Matrix4D projection;

    //program->setTexture<Texture2D>("samplerColor", texture);
    //program->bindUniform<core::Matrix4D>("projection", projection);
}

SimpleRender::~SimpleRender()
{
    delete m_pipeline;
    delete m_program;
    delete m_renderTarget;
    delete m_texture;

    //delete shaders
}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0, 3), 1);
}
} //namespace v3d
