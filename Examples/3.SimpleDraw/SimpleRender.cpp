#include "SimpleRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

#include "Renderer/Shader.h"
#include "Scene/Camera.h"
#include "Scene/Model.h"


namespace v3d
{
namespace renderer
{

SimpleRender::SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const scene::Model*> models) noexcept
    : m_camera(nullptr)
{
    m_program = cmdList.createObject<ShaderProgram>(shaders);

    m_sampler = cmdList.createObject<SamplerState>(SamplerFilter::SamplerFilter_Nearest, SamplerFilter::SamplerFilter_Nearest);
    //m_texture = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Sampled | TextureUsage_Shared | TextureUsage_Write, Format::Format_BC2_UNorm_Block, size,,,);

    m_modelDrawer = new scene::ModelHelper(cmdList, models);

    m_renderTarget = cmdList.createObject<RenderTargetState>(size);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, size, TextureSamples::TextureSamples_x1);
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(m_modelDrawer->getVertexInputAttribDescription(0, 0), m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(false);

    //cmdList.setPipelineState(m_pipeline[0]);

    //test
    /*
    std::vector<f32> vertexBuffer =
    {
        0.0f,  1.0f, 0.0f ,     1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f ,     0.0f, 1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f ,     0.0f, 0.0f, 1.0f
    };

    VertexInputAttribDescription::InputBinding binding(0, VertexInputAttribDescription::InputRate_Vertex, sizeof(f32) * 6);
    renderer::VertexInputAttribDescription vertexDesc({ binding },
        {
            { binding._index, 0, Format::Format_R32G32B32_SFloat, 0 },                  //pos
            { binding._index, 0, Format::Format_R32G32B32_SFloat, sizeof(f32) * 3 }     //color
        }
    );
    u64 vertexBufferSize = vertexBuffer.size() * sizeof(f32);
    m_drawBuffer = cmdList.createObject<VertexStreamBuffer>(StreamBufferUsage::StreamBuffer_Write | StreamBufferUsage::StreamBuffer_Shared, vertexBufferSize, (u8*)vertexBuffer.data());


    const Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.vert");
    const Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(cmdList.getContext(), "examples/3.simpledraw/shaders/simple.frag");
    const std::vector<const Shader*> sha = { vertShader, fragShader };
    m_program = cmdList.createObject<ShaderProgram>(sha);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(vertexDesc, m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(vertexDesc, m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(CullMode::CullMode_None);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);
    */
    //

    cmdList.setRenderTarget(m_renderTarget);

    cmdList.sumitCommands();
    cmdList.flushCommands();
}

SimpleRender::~SimpleRender()
{
    delete m_modelDrawer;

    delete m_renderTarget;
    delete m_pipeline;
    delete m_program;
}

void SimpleRender::updateParameters(renderer::CommandList& cmdList, const std::vector<Parameter>& parameters)
{
    struct
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
        core::Vector4D lightPos = core::Vector4D(25.0f, 5.0f, 5.0f, 1.0f);
    }
    uboVS;

     uboVS.projectionMatrix = m_camera->getProjectionMatrix();
     uboVS.viewMatrix = m_camera->getViewMatrix();

     //uboVS.modelMatrix.makeIdentity();
     //uboVS.modelMatrix.setRotation(rotate);
     //uboVS.modelMatrix.setTranslation(pos);

      m_program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(uboVS), &uboVS);
      //m_program->bindSampledTexture<Texture2D, ShaderType::ShaderType_Fragment>("samplerColorMap", m_texture, m_sampler);
}

void SimpleRender::update(renderer::CommandList& cmdList)
{
    //TODO
}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));

    if (m_modelDrawer)
    {
        m_pipeline->setDepthWrite(true);
        m_pipeline->setDepthTest(true);
        m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
        m_pipeline->setColorMask(ColorMask::ColorMask_None);
        cmdList.setPipelineState(m_pipeline);

        SimpleRender::updateParameters(cmdList, {});
        m_modelDrawer->drawModel();

        m_pipeline->setDepthWrite(false);
        m_pipeline->setDepthTest(true);
        m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Equal);
        m_pipeline->setColorMask(ColorMask::ColorMask_All);
        cmdList.setPipelineState(m_pipeline);

        SimpleRender::updateParameters(cmdList, {});
        m_modelDrawer->drawModel();
    }
}

void SimpleRender::setCamera(scene::Camera * camera)
{
    m_camera = camera;
}

} //namespace renderer
} //namespace v3d
