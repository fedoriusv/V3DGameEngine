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

SimpleRender::SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const resource::Image*> image, const std::vector<const scene::Model*> models) noexcept
    : m_camera(nullptr)
{
    m_program = cmdList.createObject<ShaderProgram>(shaders);

    if (!image.empty())
    {
        m_sampler = cmdList.createObject<SamplerState>(SamplerFilter::SamplerFilter_Bilinear, SamplerFilter::SamplerFilter_Bilinear, SamplerAnisotropic::SamplerAnisotropic_None);
        m_texture = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Sampled | TextureUsage_Shared | TextureUsage_Write,
            image[0]->getFormat(), core::Dimension2D(image[0]->getDimension().width, image[0]->getDimension().height), 1, image[0]->getRawData());
        m_images.emplace(image[0], std::make_pair(m_texture, m_sampler.get()));
    }
    m_modelDrawer = new scene::ModelHelper(cmdList, models);

    m_renderTarget = cmdList.createObject<RenderTargetState>(size);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, size, TextureSamples::TextureSamples_x1);
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(m_modelDrawer->getVertexInputAttribDescription(0, 0), m_program, m_renderTarget);
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    //m_pipeline->setPolygonMode(PolygonMode::PolygonMode_Line);
    m_pipeline->setFrontFace(FrontFace::FrontFace_CounterClockwise);
    m_pipeline->setCullMode(CullMode::CullMode_None);
    m_pipeline->setColorMask(ColorMask::ColorMask_All);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);

    cmdList.setPipelineState(m_pipeline);
    cmdList.setRenderTarget(m_renderTarget);

    cmdList.sumitCommands();
    cmdList.flushCommands();
}

SimpleRender::~SimpleRender()
{
    delete m_modelDrawer;

    //TODO should remove self
    {
        delete m_renderTarget;
        delete m_pipeline;
        delete m_program;
        delete m_texture;
        //delete m_sampler;
    }
}

void SimpleRender::updateParameter(renderer::CommandList & cmdList, const std::string& name, u32 size, const void * ubo)
{
    m_program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(name, 0, size, ubo);
}

void SimpleRender::updateParameter(renderer::CommandList & cmdList, const std::string & name, const resource::Image * image)
{
    auto& data = m_images.begin()->second; //m_images[image];
    m_program->bindSampledTexture<ShaderType::ShaderType_Fragment, Texture2D>(name, data.first, data.second);
}

void SimpleRender::render(renderer::CommandList& cmdList)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));

    if (m_modelDrawer)
    {
        m_modelDrawer->draw();
    }
}

void SimpleRender::setCamera(scene::Camera * camera)
{
    m_camera = camera;
}

} //namespace renderer
} //namespace v3d
