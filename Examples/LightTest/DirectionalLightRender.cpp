#include "DirectionalLightRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

namespace v3d
{

ForwardDirectionalLightTextureTest::ForwardDirectionalLightTextureTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

ForwardDirectionalLightTextureTest::~ForwardDirectionalLightTextureTest()
{
}

void ForwardDirectionalLightTextureTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights)
{
    m_Sampler = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/lambert/box.jpg");
    ASSERT(image, "not found");
    m_Texture = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, 1, image->getRawData(), "LambertTexture");

    std::vector<std::pair<std::string, std::string>> constants =
    {
        { "LIGHT_COUNT", std::to_string(countLights) },
    };

    renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/lambert/directionalLight.vert");
    renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/lambert/directionalLight.frag", constants);

    m_Program = m_CommandList.createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = m_CommandList.createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    m_CommandList.flushCommands();

    resource::ResourceLoaderManager::getInstance()->remove(image);
}

void ForwardDirectionalLightTextureTest::Draw(scene::ModelHelper* geometry, scene::Camera* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights)
{
    m_CommandList.setPipelineState(m_Pipeline.get());

    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D normalMatrix;
            core::Matrix4D viewMatrix;
        } ubo;

        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix = transform.getTransform();
        ubo.modelMatrix.getInverse(ubo.normalMatrix);
        ubo.normalMatrix.makeTransposed();
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
    }

    {
        struct LIGHT
        {
            core::Vector4D position;
            core::Vector4D color;
        };

        std::vector<LIGHT> light(lights.size());
        for (u32 l = 0; l < lights.size(); ++l)
        {
            light[l].position = { std::get<0>(lights[l]), 1.0 };
            light[l].color = std::get<1>(lights[l]);
        }

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "ubo" }, 0, sizeof(LIGHT) * (u32)lights.size(), light.data());
        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_Sampler.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureColor" }, m_Texture.get());
    }

    geometry->draw();
}

void ForwardDirectionalLightTextureTest::Free()
{
    m_Sampler = nullptr;
    m_Texture = nullptr;

    m_Pipeline = nullptr;
    m_Program = nullptr;
}

} //namespace v3d