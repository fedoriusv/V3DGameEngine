#include "TextureRender.h"

#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"
#include "Stream/StreamManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderCompiler.h"
#include "Resource/Loader/ShaderSourceStreamLoader.h"


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
    renderer::Descriptor vsCBO({ &_constantBufferVS, 0, sizeof(_constantBufferVS) }, 0);

    renderer::Descriptor sampler(_sampler, 1);
    renderer::Descriptor texture(renderer::TextureView(_texture), 2);
    renderer::Descriptor fsCBO({ &_constantBufferFS, 0, sizeof(_constantBufferFS) }, 3);

    cmdList.bindDescriptorSet(0, { vsCBO, fsCBO, texture, sampler });
}


TextureRender::TextureRender(renderer::Device* device, renderer::CmdListRender& cmdList, renderer::Swapchain* swapchain, const std::vector<const renderer::Shader*>& shaders, const renderer::VertexInputAttributeDesc& vertex) noexcept
{
    m_program = new renderer::ShaderProgram(device, static_cast<const renderer::VertexShader*>(shaders[0]), static_cast<const renderer::FragmentShader*>(shaders[1]));

    if (m_enableMSAA)
    {
        {
            m_renderTargetMSAA = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());

            m_colorAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Resolve | renderer::TextureUsage::TextureUsage_Sampled,
                renderer::Format::Format_R16G16B16A16_SFloat, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x2);
            m_renderTargetMSAA->setColorTexture(0, m_colorAttachment.get(),
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
                });

#if defined(PLATFORM_ANDROID)
                m_depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Resolve,
                renderer::Format::Format_D24_UNorm_S8_UInt, swapchain->getBackbufferSize(), TextureSamples::TextureSamples_x2);
#else
                m_depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment,
                renderer::Format::Format_D32_SFloat, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x2);
#endif
            m_renderTargetMSAA->setDepthStencilTexture(m_depthAttachment.get(),
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
                },
                {
                    renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
                });

            m_pipelineMSAA = new renderer::GraphicsPipelineState(device, vertex, m_renderTargetMSAA->getRenderPassDesc(), m_program.get());
            m_pipelineMSAA->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineMSAA->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_pipelineMSAA->setCullMode(renderer::CullMode::CullMode_None);
            m_pipelineMSAA->setColorMask(0, renderer::ColorMask::ColorMask_All);
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
                    renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
                });

            m_pipelineBackbuffer = new renderer::GraphicsPipelineState(device, renderer::VertexInputAttributeDesc(), m_renderTargetBackbuffer->getRenderPassDesc(), m_programBackbuffer.get(), "Backbuffer");
            m_pipelineBackbuffer->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_pipelineBackbuffer->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_pipelineBackbuffer->setCullMode(renderer::CullMode::CullMode_Back);
            m_pipelineBackbuffer->setColorMask(0, renderer::ColorMask::ColorMask_All);
            m_pipelineBackbuffer->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
            m_pipelineBackbuffer->setDepthWrite(false);
            m_pipelineBackbuffer->setDepthTest(false);
        }

        m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    }
    else
    {
        m_renderTarget = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());
        m_renderTarget->setColorTexture(0, swapchain->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
        });
#if defined(PLATFORM_ANDROID)
        m_depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment,
            renderer::Format::Format_D24_UNorm_S8_UInt, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#else
        m_depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment,
            renderer::Format::Format_D32_SFloat_S8_UInt, swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#endif
        m_renderTarget->setDepthStencilTexture(m_depthAttachment.get(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0.0f);

        m_pipeline = new renderer::GraphicsPipelineState(device, vertex, m_renderTarget->getRenderPassDesc(), m_program.get(), "TextureRender");
        m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        m_pipeline->setDepthWrite(true);
        m_pipeline->setDepthTest(true);
    }
}

TextureRender::~TextureRender()
{
}

void TextureRender::process(renderer::CmdListRender& cmdList, const std::vector<std::tuple<renderer::GeometryBufferDesc, DrawProperties>>& props)
{
    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect(0, 0, m_renderTargetMSAA->getRenderArea()._width, m_renderTargetMSAA->getRenderArea()._height));
        cmdList.setScissor(math::Rect(0, 0, m_renderTargetMSAA->getRenderArea()._width, m_renderTargetMSAA->getRenderArea()._height));
        cmdList.beginRenderTarget(*m_renderTargetMSAA.get());
        cmdList.setPipelineState(*m_pipelineMSAA.get());
    }
    else
    {
        cmdList.setViewport(math::Rect(0, 0, m_renderTarget->getRenderArea()._width, m_renderTarget->getRenderArea()._height));
        cmdList.setScissor(math::Rect(0, 0, m_renderTarget->getRenderArea()._width, m_renderTarget->getRenderArea()._height));
        cmdList.beginRenderTarget(*m_renderTarget.get());
        cmdList.setPipelineState(*m_pipeline.get());
    }

    m_shaderParameters.bindUniformParameters(cmdList, m_program.get());

    for (auto& buffer : props)
    {

        const  DrawProperties& props = std::get<1>(buffer);
        if (props._indexDraws)
        {
            cmdList.drawIndexed(std::get<0>(buffer), props._start, props._count, 0, 0, props._countInstance);
        }
        else
        {
            cmdList.draw(std::get<0>(buffer), props._start, props._count, 0, props._countInstance);
        }
    }

    cmdList.endRenderTarget();

    if (m_enableMSAA)
    {
        cmdList.setViewport(math::Rect(0, 0, m_renderTargetBackbuffer->getRenderArea()._width, m_renderTargetBackbuffer->getRenderArea()._height));
        cmdList.setScissor(math::Rect(0, 0, m_renderTargetBackbuffer->getRenderArea()._width, m_renderTargetBackbuffer->getRenderArea()._height));
        cmdList.beginRenderTarget(*m_renderTargetBackbuffer.get());
        cmdList.setPipelineState(*m_pipelineBackbuffer.get());

        renderer::Descriptor colorTexture(m_renderTargetMSAA->getColorTexture<renderer::Texture2D>(0), 0);
        renderer::Descriptor colorSampler(m_sampler.get(), 1);
        cmdList.bindDescriptorSet(0, { colorSampler, colorTexture });

        cmdList.draw(renderer::GeometryBufferDesc(nullptr, 0, 0), 0, 3, 0, 1);

        cmdList.endRenderTarget();
    }
}

void TextureRender::updateParameters(renderer::CmdListRender& cmdList, const std::function<void(TextureUniformParameters&)>& callback)
{
    callback(m_shaderParameters);
}

} //namespace v3d
