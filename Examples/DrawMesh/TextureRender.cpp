#include "TextureRender.h"

#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Core/Context.h"
#include "Stream/StreamManager.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

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
    program->bindUniformsBuffer<ShaderType::Vertex>({ "ubo" }, 0, sizeof(UniformBuffer), &_constantBufferVS);

    program->bindUniformsBuffer<ShaderType::Fragment>({ "light" }, 0, sizeof(Light), &_constantBufferFS);
    program->bindTexture<ShaderType::Fragment, Texture2D>({ "textureColor" }, _texture);
    program->bindSampler<ShaderType::Fragment>({ "samplerColor" }, _sampler);
}


TextureRender::TextureRender(CommandList& cmdList, const v3d::math::Dimension2D& viewport, const std::vector<const Shader*>& shaders, const VertexInputAttributeDescription& vertex) noexcept
{
    m_program = cmdList.createObject<ShaderProgram>(shaders);

    if (m_enableMSAA)
    {
        {
            m_renderTargetMSAA = cmdList.createObject<RenderTargetState>(viewport);

            Texture2D* colorAttachmentMSAA = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Resolve | TextureUsage::TextureUsage_Sampled, Format::Format_R16G16B16A16_SFloat, viewport, TextureSamples::TextureSamples_x2);
            m_renderTargetMSAA->setColorTexture(0, colorAttachmentMSAA,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
                });

#if defined(PLATFORM_ANDROID)
            Texture2D* depthAttachmentMSAA = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Resolve, Format::Format_D24_UNorm_S8_UInt, viewport, TextureSamples::TextureSamples_x2);
#else
            Texture2D* depthAttachmentMSAA = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat, viewport, TextureSamples::TextureSamples_x2);
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

            m_pipelineMSAA = cmdList.createObject<GraphicsPipelineState>(vertex, m_program.get(), m_renderTargetMSAA.get());
            m_pipelineMSAA->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineMSAA->setFrontFace(FrontFace::FrontFace_Clockwise);
            m_pipelineMSAA->setCullMode(CullMode::CullMode_None);
            m_pipelineMSAA->setColorMask(ColorMask::ColorMask_All);
            m_pipelineMSAA->setDepthCompareOp(CompareOperation::CompareOp_GreaterOrEqual);
            m_pipelineMSAA->setDepthWrite(true);
            m_pipelineMSAA->setDepthTest(true);
        }

        {
            const renderer::Shader* vertexShader = nullptr;
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
                renderer::ShaderHeader vertexHeader(renderer::ShaderType::Vertex);
                vertexHeader._contentType = renderer::ShaderHeader::ShaderContent::Source;
                vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;

                vertexShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(cmdList.getContext(), "vertex", &vertexHeader, vertexStream,
                    "main", {}, {}, resource::ShaderSourceStreamLoader::ShaderSourceFlag::ShaderSource_UseLegacyCompilerForHLSL);
                delete vertexStream;
            }

            const renderer::Shader* fragmentShader = nullptr;
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
                renderer::ShaderHeader fragmentHeader(renderer::ShaderType::Fragment);
                fragmentHeader._contentType = renderer::ShaderHeader::ShaderContent::Source;
                fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;

                fragmentShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(cmdList.getContext(), "fragment", &fragmentHeader, fragmentStream,
                    "main", {}, {}, resource::ShaderSourceStreamLoader::ShaderSourceFlag::ShaderSource_UseLegacyCompilerForHLSL);
                delete fragmentStream;

                m_programBackbuffer = cmdList.createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertexShader, fragmentShader });
            }

            m_renderTargetBackbuffer = cmdList.createObject<RenderTargetState>(viewport);
            m_renderTargetBackbuffer->setColorTexture(0, cmdList.getBackbuffer(),
                {
                    renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
                });

            m_pipelineBackbuffer = cmdList.createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttributeDescription(), m_programBackbuffer.get(), m_renderTargetBackbuffer.get());
            m_pipelineBackbuffer->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineBackbuffer->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_pipelineBackbuffer->setCullMode(renderer::CullMode::CullMode_Back);
            m_pipelineBackbuffer->setColorMask(renderer::ColorMask::ColorMask_All);
            m_pipelineBackbuffer->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
            m_pipelineBackbuffer->setDepthWrite(false);
            m_pipelineBackbuffer->setDepthTest(false);
        }

        m_Sampler = cmdList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    }
    else
    {
        m_renderTarget = cmdList.createObject<RenderTargetState>(viewport);
        m_renderTarget->setColorTexture(0, cmdList.getBackbuffer(), RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f));
#if defined(PLATFORM_ANDROID)
        Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D24_UNorm_S8_UInt, viewport, TextureSamples::TextureSamples_x1);
#else
        Texture2D* depthAttachment = cmdList.createObject<Texture2D>(TextureUsage::TextureUsage_Attachment, Format::Format_D32_SFloat_S8_UInt, viewport, TextureSamples::TextureSamples_x1);
#endif
        m_renderTarget->setDepthStencilTexture(depthAttachment, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_DontCare, 0.0f);

        m_pipeline = cmdList.createObject<GraphicsPipelineState>(vertex, m_program.get(), m_renderTarget.get());
        m_pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_pipeline->setFrontFace(FrontFace::FrontFace_Clockwise);
        m_pipeline->setCullMode(CullMode::CullMode_None);
        m_pipeline->setColorMask(ColorMask::ColorMask_All);
        m_pipeline->setDepthCompareOp(CompareOperation::CompareOp_GreaterOrEqual);
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
            Texture2D* colorAttachment = m_renderTargetMSAA->getColorTexture<Texture2D>(0);
            delete colorAttachment;
        }

        if (m_renderTargetMSAA->hasDepthStencilTexture())
        {
            Texture2D* depthAttachment = m_renderTargetMSAA->getDepthStencilTexture<Texture2D>();
            delete depthAttachment;
        }
    }
    else
    {
        if (m_renderTarget->hasDepthStencilTexture())
        {
            Texture2D* depthAttachment = m_renderTarget->getDepthStencilTexture<Texture2D>();
            delete depthAttachment;
        }
    }
}

void TextureRender::process(renderer::CommandList& cmdList, const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& props)
{
    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTargetMSAA->getDimension().m_width, m_renderTargetMSAA->getDimension().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTargetMSAA->getDimension().m_width, m_renderTargetMSAA->getDimension().m_height));
        cmdList.setRenderTarget(m_renderTargetMSAA.get());
        cmdList.setPipelineState(m_pipelineMSAA.get());
    }
    else
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTarget->getDimension().m_width, m_renderTarget->getDimension().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTarget->getDimension().m_width, m_renderTarget->getDimension().m_height));
        cmdList.setRenderTarget(m_renderTarget.get());
        cmdList.setPipelineState(m_pipeline.get());
    }

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

    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect32(0, 0, m_renderTargetBackbuffer->getDimension().m_width, m_renderTargetBackbuffer->getDimension().m_height));
        cmdList.setScissor(math::Rect32(0, 0, m_renderTargetBackbuffer->getDimension().m_width, m_renderTargetBackbuffer->getDimension().m_height));
        cmdList.setRenderTarget(m_renderTargetBackbuffer.get());
        cmdList.setPipelineState(m_pipelineBackbuffer.get());

        m_programBackbuffer->bindSampler<renderer::ShaderType::Fragment>({ "colorSampler" }, m_Sampler.get());
        m_programBackbuffer->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "colorTexture" }, m_renderTargetMSAA->getColorTexture<Texture2D>(0));

        cmdList.draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
    }
}

void TextureRender::updateParameters(renderer::CommandList& cmdList, const std::function<void(TextureUniformParameters&)>& callback)
{
    callback(m_shaderParameters);
}

} //namespace v3d
