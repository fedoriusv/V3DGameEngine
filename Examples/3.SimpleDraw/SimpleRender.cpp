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
    : m_drawBuffer(nullptr)
    , m_camera(nullptr)
{
    m_program[0] = cmdList.createObject<ShaderProgram>(shaders);

    m_modelDrawer = new scene::ModelHelper(cmdList, models);

    m_renderTarget = cmdList.createObject<RenderTarget>(size);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, size, TextureSamples::TextureSamples_x1);
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_pipeline[0] = cmdList.createObject<GraphicsPipelineState>(m_modelDrawer->getVertexInputAttribDescription(0, 0), m_program[0], m_renderTarget);
    m_pipeline[0]->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline[0]->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline[0]->setCullMode(CullMode::CullMode_None);
    m_pipeline[0]->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline[0]->setDepthWrite(true);
    m_pipeline[0]->setDepthTest(true);

    //cmdList.setPipelineState(m_pipeline[0]);

    //test
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
    m_program[1] = cmdList.createObject<ShaderProgram>(sha);

    m_pipeline[1] = cmdList.createObject<GraphicsPipelineState>(vertexDesc, m_program[1], m_renderTarget);
    m_pipeline[1]->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline[1]->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline[1]->setCullMode(CullMode::CullMode_None);
    m_pipeline[1]->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline[1]->setDepthWrite(true);
    m_pipeline[1]->setDepthTest(true);

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

void SimpleRender::update(u32 shaderIndex, renderer::CommandList& cmdList, const core::Vector3D& pos, const core::Vector3D& rotate)
{
    if (shaderIndex == 0)
    {
        struct
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D viewMatrix;
            core::Vector4D lightPos = core::Vector4D(25.0f, 5.0f, 5.0f, 1.0f);
        }
        uboVS, uboVS1;

        uboVS.projectionMatrix = m_camera->getProjectionMatrix();
        uboVS.viewMatrix = m_camera->getViewMatrix();

        uboVS.modelMatrix.makeIdentity();
        uboVS.modelMatrix.setRotation(rotate);
        uboVS.modelMatrix.setTranslation(pos);
        //uboVS.modelMatrix.makeTransposed();

        m_program[shaderIndex]->bindUniformsBuffer<ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(uboVS), &uboVS);
        //m_program->bindTexture<Texture2D, ShaderType::ShaderType_Fragment>("samplerColorMap", m_colorTexture);
    }
    else
    {
        struct
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D viewMatrix;
        }
        uboVS, uboVS1;

        uboVS.projectionMatrix = m_camera->getProjectionMatrix();
        uboVS.viewMatrix = m_camera->getViewMatrix();

        uboVS.modelMatrix.makeIdentity();
        uboVS.modelMatrix.setRotation(rotate);
        uboVS.modelMatrix.setTranslation(pos);
        //uboVS.modelMatrix.makeTransposed();

        m_program[shaderIndex]->bindUniformsBuffer<ShaderType::ShaderType_Vertex>("ubo", 0, sizeof(uboVS), &uboVS);
        //m_program->bindTexture<Texture2D, ShaderType::ShaderType_Fragment>("samplerColorMap", m_colorTexture);
    }

}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));

    if (m_modelDrawer)
    {
        cmdList.setPipelineState(m_pipeline[0]);
        SimpleRender::update(0, cmdList, core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f));
        m_modelDrawer->drawModel();
    }

    if (m_drawBuffer)
    {
        cmdList.setPipelineState(m_pipeline[1]);
        SimpleRender::update(1, cmdList, core::Vector3D(0.0f, 0.0f, 5.1f), core::Vector3D(0.0f, 0.0f, 40.0f));
        cmdList.draw(renderer::StreamBufferDescription(m_drawBuffer, 0), 0, 3, 1);
        SimpleRender::update(1, cmdList, core::Vector3D(0.0f, 0.0f, 3.0f), core::Vector3D(0.0f));
        cmdList.draw(renderer::StreamBufferDescription(m_drawBuffer, 0), 0, 3, 1);
    }
}

void SimpleRender::setCamera(scene::Camera * camera)
{
    m_camera = camera;
}

} //namespace renderer
} //namespace v3d
