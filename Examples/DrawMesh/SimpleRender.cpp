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
		ASSERT(image[0], "nullptr");
        m_sampler = cmdList.createObject<SamplerState>(SamplerFilter::SamplerFilter_Bilinear, SamplerFilter::SamplerFilter_Bilinear, SamplerAnisotropic::SamplerAnisotropic_None);
        m_texture = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Sampled | TextureUsage_Shared | TextureUsage_Write,
            image[0]->getFormat(), core::Dimension2D(image[0]->getDimension().width, image[0]->getDimension().height), 1, 1, image[0]->getRawData());
    }
    m_modelDrawer = new scene::ModelHelper(cmdList, models);

    m_renderTarget = cmdList.createObject<RenderTargetState>(size);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, size, TextureSamples::TextureSamples_x1);
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(m_modelDrawer->getVertexInputAttribDescription(0, 0), m_program.get(), m_renderTarget.get());
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_CounterClockwise);
    m_pipeline->setCullMode(CullMode::CullMode_None);
    m_pipeline->setColorMask(ColorMask::ColorMask_All);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);

    cmdList.setPipelineState(m_pipeline.get());
    cmdList.setRenderTarget(m_renderTarget.get());

    cmdList.sumitCommands();
    cmdList.flushCommands();
}

SimpleRender::~SimpleRender()
{
    Texture2D* depthAttachment = m_renderTarget->getDepthStencilTexture();
    if (depthAttachment)
    {
        delete depthAttachment;
    }

    delete m_modelDrawer;
}

void SimpleRender::updateParameter(renderer::CommandList & cmdList, const std::string& name, u32 size, const void * ubo)
{
    m_program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(0/*name*/, 0, size, ubo);
}

void SimpleRender::updateParameter(renderer::CommandList & cmdList, const std::string & name, u32 index)
{
    m_program->bindSampledTexture<ShaderType::ShaderType_Fragment, Texture2D>(0/*name*/, m_texture.get(), m_sampler.get());
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