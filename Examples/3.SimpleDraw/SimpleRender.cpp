#include "SimpleRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"


namespace v3d
{
namespace renderer
{

SimpleRender::SimpleRender(renderer::CommandList& cmdList, const renderer::VertexInputAttribDescription& desc, const std::vector<f32>& geomentry)
{
    resource::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.vert");
    resource::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<resource::Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.frag");
    m_program = cmdList.createObject<ShaderProgram>(std::vector<resource::Shader*>{vertShader, fragShader});

    m_texture = cmdList.createObject<Texture2D>(Format::Format_R8G8B8A8_UInt, core::Dimension2D(1024, 768), TextureSamples::TextureSamples_x1);
    m_renderTarget = cmdList.createObject<RenderTarget>(m_texture->getDimension());
    bool success = m_renderTarget->setColorTexture(0, m_texture, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store);

    u64 vertexBufferSize = geomentry.size() * sizeof(f32);
    m_vetexBuffer = cmdList.createObject<VertexStreamBuffer>(StreamBufferUsage::StreamBuffer_Write | StreamBufferUsage::StreamBuffer_Shared, vertexBufferSize, geomentry.data());

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(desc, m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);

    cmdList.setPipelineState(m_pipeline);
    cmdList.setRenderTarget(m_renderTarget);

    core::Matrix4D projection;
    renderer::Texture2D* texture;

    m_program->bindTexture<renderer::Texture2D, resource::ShaderType::ShaderType_Fragment>("samplerColor", texture);
    //m_program->bindUniform<core::Matrix4D, Vertex>("projection", projection);
    //m_program->bindUniformsBuffer<Vertex>("ubo", nullptr, 0, 0);
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
    //cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, 0, 3), 1);
    //cmdList.draw(renderer::StreamBufferDescriptions({ m_vetexBuffer0, 0, 3) }, { { m_vetexBuffer1, 0, 3) }, 1);
    //cmdList.drawIndexed(renderer::StreamBufferDescription(m_vetexBuffer, 0, 3), StreamIndexDescription( m_indexBuffer, 0, 3), 1);
    //

    //cmdList.setStreamBuffer(renderer::StreamVertexBufferDesc(m_vetexBuffer, offset, size));
    //cmdList.setStreamBuffer({ renderer::StreamVertexBufferDesc(m_vetexBuffer0, offset, size), renderer::StreamVertexBufferDesc(m_vetexBuffer1, offset, size) });

    //m_program->bindUniform<core::Matrix4D>("pos", matrix);
    //cmdList.draw(0, 3, 1);
    //m_program->bindUniform<core::Matrix4D>("pos", matrix);
    //cmdList.draw(0, 3, 1);

    ////////////////////////////////////

    //m_program->bindUniform<core::Matrix4D>("pos", matrix);
    //cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, offset, size), 0, 3, 1); !!!!!!!!
    ////cmdList.draw(renderer::StreamBufferDescriptions({ m_vetexBuffer0, offset, size }, m_vetexBuffer1, 0, 3) }, 0, 3, 1);
    //m_program->bindUniform<core::Matrix4D>("pos", matrix);
    //cmdList.draw(renderer::StreamBufferDescription(m_vetexBuffer, offset, size), 0, 3, 1);

    //viod* m_data = m_unifrom->map();
    //m_program->bindUniformsBuffer("ubo", data, offset, size));
}

} //namespace renderer
} //namespace v3d
