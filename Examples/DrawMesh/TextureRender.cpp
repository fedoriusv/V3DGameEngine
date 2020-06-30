#include "TextureRender.h"

#include "Renderer/Context.h"
#include "Renderer/Object/Texture.h"

namespace v3d
{

using namespace renderer;

TextureUniformParameters::TextureUniformParameters() noexcept
    : _texture(nullptr)
    , _sampler(nullptr)
{
}

void TextureUniformParameters::bindUniformParameters(CommandList& cmdList, ShaderProgram* program)
{
#if USE_STRING_ID_SHADER
    program->bindUniformsBuffer<ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UniformBuffer), &_constantBuffer);

    program->bindTexture<ShaderType::ShaderType_Fragment, Texture2D>({ "textureColor" }, _texture);
    program->bindSampler<ShaderType::ShaderType_Fragment>({ "samplerColor" }, _sampler);
#else
    ASSERT(false, "not impl");
#endif
}


TextureRender::TextureRender(CommandList& cmdList, const std::vector<const Shader*>& shaders, const VertexInputAttribDescription& vertex) noexcept
{
    m_program = cmdList.createObject<ShaderProgram>(shaders);

    core::Dimension2D size = cmdList.getContext()->getBackbufferSize();
    m_renderTarget = cmdList.createObject<RenderTargetState>(size);
    m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
#if defined(PLATFORM_ANDROID)
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D24_UNorm_S8_UInt, size, TextureSamples::TextureSamples_x1);
#else
    Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, size, TextureSamples::TextureSamples_x1);
#endif
    m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_pipeline = cmdList.createObject<GraphicsPipelineState>(vertex, m_program.get(), m_renderTarget.get());
    m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(CullMode::CullMode_None);
    m_pipeline->setColorMask(ColorMask::ColorMask_All);
    m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_Less);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);
}

TextureRender::~TextureRender()
{
    Texture2D* depthAttachment = m_renderTarget->getDepthStencilTexture();
    if (depthAttachment)
    {
        delete depthAttachment;
    }
}

void TextureRender::process(renderer::CommandList& cmdList, const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& props)
{
    cmdList.setViewport(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_renderTarget->getDimension().width, m_renderTarget->getDimension().height));
    cmdList.setRenderTarget(m_renderTarget.get());

    cmdList.setPipelineState(m_pipeline.get());
    m_shaderParameters.bindUniformParameters(cmdList, m_program.get());

    for (auto& buffer : props)
    {
        const  renderer::DrawProperties& props = std::get<1>(buffer);
        if (props._indexDraws)
        {
            cmdList.drawIndexed(std::get<0>(buffer), props._start, props._count, props._countInstance);
        }
        else
        {
            cmdList.draw(std::get<0>(buffer), props._start, props._count, props._countInstance);
        }
    }
}

void TextureRender::updateParameters(renderer::CommandList& cmdList, const std::function<void(TextureUniformParameters&)>& callback)
{
    callback(m_shaderParameters);
}

} //namespace v3d
