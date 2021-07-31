#include "UnlitRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

namespace v3d
{

UnlitTextureTest::UnlitTextureTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

UnlitTextureTest::~UnlitTextureTest()
{
}

void UnlitTextureTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc)
{
    m_Sampler = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/unlit/basetex.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(image, "not found");
    m_Texture = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), image->getMipMapsCount(), image->getRawData(), "UnlitTexture");

    //const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/unlit/texture.vert");
    //const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/unlit/texture.frag");
    //std::vector<const renderer::Shader*> shaders = { vertShader, fragShader };
    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/unlit/texture.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_PS", renderer::ShaderType::Fragment }

        });

    m_Program = m_CommandList.createObject<renderer::ShaderProgram>(shaders);
    m_Pipeline = m_CommandList.createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    m_CommandList.flushCommands();

    resource::ResourceLoaderManager::getInstance()->remove(image);
}

void UnlitTextureTest::Draw(scene::ModelHelper* geometry, scene::CameraHelper* camera, const v3d::scene::Transform& transform)
{
    m_CommandList.setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    ubo.projectionMatrix = camera->getProjectionMatrix();
    ubo.modelMatrix = transform.getTransform();
    ubo.viewMatrix = camera->getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);
    m_Program->bindSampler<renderer::ShaderType::Fragment>({ "samplerColor" }, m_Sampler.get());
    m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureColor" }, m_Texture.get());

    geometry->draw(&m_CommandList);
}

void UnlitTextureTest::Free()
{
    m_Sampler = nullptr;
    m_Texture = nullptr;

    m_Pipeline = nullptr;
    m_Program = nullptr;
}

} //namespace v3d