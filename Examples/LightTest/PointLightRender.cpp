#include "PointLightRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

#include "Scene/CameraHelper.h"

namespace v3d
{

ForwardPointLightTest::ForwardPointLightTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

ForwardPointLightTest::~ForwardPointLightTest()
{
}

void ForwardPointLightTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights)
{
    std::vector<std::pair<std::string, std::string>> constants =
    {
        { "CONSTANT", std::to_string(1.0) },
        { "LINEAR", std::to_string(0.09) },
        { "QUADRATIC", std::to_string(0.032) },

        { "LIGHT_COUNT", std::to_string(countLights) }
    };

    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/point/phongTextureless.vert");
    const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/point/phongTextureless.frag", constants);

    m_Program = m_CommandList.createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = m_CommandList.createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
}

void ForwardPointLightTest::Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights)
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

        struct UBO
        {
            core::Vector4D viewPosition;
            core::Vector4D diffuse;
            core::Vector4D specular;
        } ubo;

        ubo.viewPosition = { camera->getViewPosition(), 1.0 };
        ubo.diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
        ubo.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
        
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "light" }, 0, sizeof(LIGHT) * (u32)light.size(), light.data());
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "ubo" }, 0, sizeof(UBO), &ubo);
    }

    geometry->draw();
}

void ForwardPointLightTest::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;
}


ForwardPointLightTextureTest::ForwardPointLightTextureTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

ForwardPointLightTextureTest::~ForwardPointLightTextureTest()
{
}

void ForwardPointLightTextureTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights)
{
    m_Sampler = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    resource::Image* imageDiffuse = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/point/container2.png", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageDiffuse, "not found");
    m_TextureDiffuse = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
        imageDiffuse->getFormat(), core::Dimension2D(imageDiffuse->getDimension().width, imageDiffuse->getDimension().height), imageDiffuse->getMipMapsCount(), imageDiffuse->getRawData(), "PhongDiffuse");

    resource::Image* imageSpecular = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/point/container2_specular.png", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageSpecular, "not found");
    m_TextureSpecular = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
        imageSpecular->getFormat(), core::Dimension2D(imageSpecular->getDimension().width, imageSpecular->getDimension().height), imageSpecular->getMipMapsCount(), imageSpecular->getRawData(), "PhongSpecular");

    std::vector<std::pair<std::string, std::string>> constants =
    {
        { "CONSTANT", std::to_string(1.0) },
        { "LINEAR", std::to_string(0.09) },
        { "QUADRATIC", std::to_string(0.032) },

        { "LIGHT_COUNT", std::to_string(countLights) }
    };

    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/point/phong.vert");
    const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/point/phong.frag", constants);

    m_Program = m_CommandList.createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = m_CommandList.createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    m_CommandList.flushCommands();

    resource::ResourceLoaderManager::getInstance()->remove(imageDiffuse);
    resource::ResourceLoaderManager::getInstance()->remove(imageSpecular);
}

void ForwardPointLightTextureTest::Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights)
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

        struct UBO
        {
            core::Vector4D viewPosition;
        } ubo;

        ubo.viewPosition = { camera->getViewPosition(), 1.0 };

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "light" }, 0, sizeof(LIGHT) * (u32)light.size(), light.data());
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "ubo" }, 0, sizeof(UBO), &ubo);

        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_Sampler.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureDiffuse" }, m_TextureDiffuse.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureSpecular" }, m_TextureSpecular.get());
    }

    geometry->draw();
}

void ForwardPointLightTextureTest::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    m_Sampler = nullptr;
    m_TextureDiffuse = nullptr;
    m_TextureSpecular = nullptr;
}

} //namespace v3d