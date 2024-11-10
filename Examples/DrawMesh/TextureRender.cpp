#include "TextureRender.h"

#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"
#include "Stream/StreamManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderCompiler.h"
#include "Resource/ShaderSourceStreamLoader.h"


using namespace v3d;

namespace app
{

TextureUniformParameters::TextureUniformParameters() noexcept
    : _texture(nullptr)
    , _sampler(nullptr)
{
}

void TextureUniformParameters::bindUniformParameters(renderer::CmdListRender& cmdList, renderer::ShaderProgram* program)
{
    //program->bindUniformsBuffer<ShaderType::Vertex>({ "ubo" }, 0, sizeof(UniformBuffer), &_constantBufferVS);

    //program->bindUniformsBuffer<ShaderType::Fragment>({ "light" }, 0, sizeof(Light), &_constantBufferFS);
    //program->bindTexture<ShaderType::Fragment, Texture2D>({ "textureColor" }, _texture);
    //program->bindSampler<ShaderType::Fragment>({ "samplerColor" }, _sampler);
}


TextureRender::TextureRender(renderer::Device* device, renderer::CmdListRender& cmdList, renderer::Swapchain* swapchain, const std::vector<const renderer::Shader*>& shaders, const renderer::VertexInputAttributeDesc& vertex) noexcept
{
    m_program = new renderer::ShaderProgram(device, static_cast<const renderer::VertexShader*>(shaders[0]), static_cast<const renderer::FragmentShader*>(shaders[1]));

    if (m_enableMSAA)
    {
        {
            m_renderTargetMSAA = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());

            renderer::Texture2D* colorAttachmentMSAA = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Resolve | renderer::TextureUsage::TextureUsage_Sampled,
                renderer::Format::Format_R16G16B16A16_SFloat, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x2);
            m_renderTargetMSAA->setColorTexture(0, colorAttachmentMSAA,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
                });

#if defined(PLATFORM_ANDROID)
            renderer::Texture2D* depthAttachmentMSAA = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Resolve,
                renderer::Format::Format_D24_UNorm_S8_UInt, swapchain->getBackbufferSize(), TextureSamples::TextureSamples_x2);
#else
            renderer::Texture2D* depthAttachmentMSAA = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment,
                renderer::Format::Format_D32_SFloat, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x2);
#endif
            m_renderTargetMSAA->setDepthStencilTexture(depthAttachmentMSAA,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
                },
                {
                    renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
                });

            m_pipelineMSAA = new renderer::GraphicsPipelineState(device, vertex, m_program.get(), m_renderTargetMSAA.get());
            m_pipelineMSAA->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineMSAA->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_pipelineMSAA->setCullMode(renderer::CullMode::CullMode_None);
            m_pipelineMSAA->setColorMask(renderer::ColorMask::ColorMask_All);
            m_pipelineMSAA->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
            m_pipelineMSAA->setDepthWrite(true);
            m_pipelineMSAA->setDepthTest(true);
        }

        {
            const renderer::VertexShader* vertexShader = nullptr;
            {
                const std::string vertexSource("\
                    struct VS_OUTPUT\n\
                    {\n\
                        float4 Pos : SV_Position;\n\
                        float2 UV : TEXTURE;\n\
                    };\n\
                    \n\
                    VS_OUTPUT main(uint vertexID : SV_VertexID)\n\
                    {\n\
                        VS_OUTPUT output;\n\
                        output.UV = float2((vertexID << 1) & 2, vertexID & 2);\n\
                        output.Pos = float4(output.UV.x * 2.0f - 1.0f, -(output.UV.y * 2.0f - 1.0f), 0.0f, 1.0f);\n\
                        return output;\n\
                    }");
                const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

                resource::ShaderDecoder::ShaderPolicy vertexPolicy;
                vertexPolicy._type = renderer::ShaderType::Vertex;
                vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
                vertexPolicy._content = renderer::ShaderContent::Source;
                vertexPolicy._entryPoint = "main";

                vertexShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(device, "vertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
                stream::StreamManager::destroyStream(vertexStream);
            }

            const renderer::FragmentShader* fragmentShader = nullptr;
            {
                const std::string fragmentSource("\
                    struct VS_OUTPUT\n\
                    {\n\
                        float4 Pos : SV_Position;\n\
                        float2 UV : TEXTURE;\n\
                    };\n\
                    SamplerState colorSampler : register(s0, space0);\n\
                    Texture2D colorTexture : register(t1, space0);\n\
                    \n\
                    float4 main(VS_OUTPUT input) : SV_TARGET0\n\
                    {\n\
                        float4 outFragColor = colorTexture.Sample(colorSampler, input.UV);\n\
                        return outFragColor;\n\
                    }");
                const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

                resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
                fragmentPolicy._type = renderer::ShaderType::Fragment;
                fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
                fragmentPolicy._content = renderer::ShaderContent::Source;
                fragmentPolicy._entryPoint = "main";

                fragmentShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(device, "fragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
                stream::StreamManager::destroyStream(fragmentStream);

                m_programBackbuffer = new renderer::ShaderProgram(device, vertexShader, fragmentShader );
            }

            m_renderTargetBackbuffer = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());
            m_renderTargetBackbuffer->setColorTexture(0, swapchain->getBackbuffer(),
                {
                    renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
                });

            m_pipelineBackbuffer = new renderer::GraphicsPipelineState(device, renderer::VertexInputAttributeDesc(), m_programBackbuffer.get(), m_renderTargetBackbuffer.get());
            m_pipelineBackbuffer->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineBackbuffer->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_pipelineBackbuffer->setCullMode(renderer::CullMode::CullMode_Back);
            m_pipelineBackbuffer->setColorMask(renderer::ColorMask::ColorMask_All);
            m_pipelineBackbuffer->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
            m_pipelineBackbuffer->setDepthWrite(false);
            m_pipelineBackbuffer->setDepthTest(false);
        }

        m_Sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    }
    else
    {
        m_renderTarget = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());
        m_renderTarget->setColorTexture(0, swapchain->getBackbuffer(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f));
#if defined(PLATFORM_ANDROID)
        renderer::Texture2D* depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment, 
            renderer::Format::Format_D24_UNorm_S8_UInt, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#else
        renderer::Texture2D* depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment, 
            renderer::Format::Format_D32_SFloat_S8_UInt, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#endif
        m_renderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0.0f);

        m_pipeline = new renderer::GraphicsPipelineState(device, vertex, m_program.get(), m_renderTarget.get());
        m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        m_pipeline->setDepthWrite(true);
        m_pipeline->setDepthTest(true);
    }
}

TextureRender::~TextureRender()
{
    if (m_enableMSAA)
    {
        if (m_renderTargetMSAA->getColorTextureCount() > 0)
        {
            renderer::Texture2D* colorAttachment = m_renderTargetMSAA->getColorTexture<renderer::Texture2D>(0);
            delete colorAttachment;
        }

        if (m_renderTargetMSAA->hasDepthStencilTexture())
        {
            renderer::Texture2D* depthAttachment = m_renderTargetMSAA->getDepthStencilTexture<renderer::Texture2D>();
            delete depthAttachment;
        }
    }
    else
    {
        if (m_renderTarget->hasDepthStencilTexture())
        {
            renderer::Texture2D* depthAttachment = m_renderTarget->getDepthStencilTexture<renderer::Texture2D>();
            delete depthAttachment;
        }
    }
}

void TextureRender::process(renderer::CmdListRender& cmdList, const std::vector<std::tuple<renderer::GeometryBufferDesc, DrawProperties>>& props)
{
    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTargetMSAA->getRenderArea().m_width, m_renderTargetMSAA->getRenderArea().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTargetMSAA->getRenderArea().m_width, m_renderTargetMSAA->getRenderArea().m_height));
        cmdList.beginRenderTarget(*m_renderTargetMSAA.get());
        cmdList.setPipelineState(*m_pipelineMSAA.get());
    }
    else
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTarget->getRenderArea().m_width, m_renderTarget->getRenderArea().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTarget->getRenderArea().m_width, m_renderTarget->getRenderArea().m_height));
        cmdList.beginRenderTarget(*m_renderTarget.get());
        cmdList.setPipelineState(*m_pipeline.get());
    }

    m_shaderParameters.bindUniformParameters(cmdList, m_program.get());

    for (auto& buffer : props)
    {

        const  DrawProperties& props = std::get<1>(buffer);
        if (props._indexDraws)
        {
            cmdList.drawIndexed(std::get<0>(buffer), props._start, props._count, 0, props._countInstance);
        }
        else
        {
            cmdList.draw(std::get<0>(buffer), props._start, props._count, 0, props._countInstance);
        }
    }

    cmdList.endRenderTarget();

    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTargetBackbuffer->getRenderArea().m_width, m_renderTargetBackbuffer->getRenderArea().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTargetBackbuffer->getRenderArea().m_width, m_renderTargetBackbuffer->getRenderArea().m_height));
        cmdList.beginRenderTarget(*m_renderTargetBackbuffer.get());
        cmdList.setPipelineState(*m_pipelineBackbuffer.get());

        //m_programBackbuffer->bindSampler<renderer::ShaderType::Fragment>({ "colorSampler" }, m_Sampler.get());
        //m_programBackbuffer->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "colorTexture" }, m_renderTargetMSAA->getColorTexture<Texture2D>(0));

        cmdList.draw(renderer::GeometryBufferDesc(nullptr, 0, 0), 0, 3, 0, 1);

        cmdList.endRenderTarget();
    }
}

void TextureRender::updateParameters(renderer::CmdListRender& cmdList, const std::function<void(TextureUniformParameters&)>& callback)
{
    callback(m_shaderParameters);
}

} //namespace v3d
