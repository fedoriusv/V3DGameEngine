#include "BumpmapRender.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

namespace v3d
{

ForwardNormalMapTest::ForwardNormalMapTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

ForwardNormalMapTest::~ForwardNormalMapTest()
{
}

void ForwardNormalMapTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights)
{
    resource::Image* imageColor = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/bumpmap/brickwall.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageColor, "not found");
    m_TextureColor = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,imageColor->getFormat(), core::Dimension2D(imageColor->getDimension().width, imageColor->getDimension().height), imageColor->getMipMapsCount(), imageColor->getRawData(), "DiffuseColor");
    m_SamplerColor = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);

    resource::Image* imageNormal = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/bumpmap/brickwall_normal.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageNormal, "not found");
    m_TextureNormalmap = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, imageNormal->getFormat(), core::Dimension2D(imageNormal->getDimension().width, imageNormal->getDimension().height), imageNormal->getMipMapsCount(), imageNormal->getRawData(), "NormalMap");
    m_SamplerNormalmap = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);

    std::vector<std::pair<std::string, std::string>> constants =
    {
        { "LIGHT_COUNT", std::to_string(countLights) }
    };

    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/normalmap.vert");
    const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/normalmap.frag", constants);
    std::vector<const renderer::Shader*> shaders = { vertShader, fragShader };
    //std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/normalmap.hlsl",
    //    {
    //        {"main_VS", renderer::ShaderType::Vertex },
    //        {"main_PS", renderer::ShaderType::Fragment }

    //    }, constants);

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

    resource::ResourceLoaderManager::getInstance()->remove(imageColor);
    resource::ResourceLoaderManager::getInstance()->remove(imageNormal);
}

void ForwardNormalMapTest::Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights)
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

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
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

        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "light" }, 0, sizeof(LIGHT) * (u32)light.size(), light.data());
        m_Program->bindSampler<renderer::ShaderType::Fragment>({ "samplerColor" }, m_SamplerColor.get());
        m_Program->bindSampler<renderer::ShaderType::Fragment>({ "samplerNormal" }, m_SamplerNormalmap.get());
        m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureColor" }, m_TextureColor.get());
        m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureNormal" }, m_TextureNormalmap.get());
    }

    geometry->draw(&m_CommandList);
}

void ForwardNormalMapTest::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    m_SamplerColor = nullptr;
    m_TextureColor = nullptr;
    m_SamplerNormalmap = nullptr;
    m_TextureNormalmap = nullptr;
}


ForwardParallaxMappingTest::ForwardParallaxMappingTest(renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
{
}

ForwardParallaxMappingTest::~ForwardParallaxMappingTest()
{
}

void ForwardParallaxMappingTest::Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights)
{
    resource::Image* imageColor = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/bumpmap/bricks2.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageColor, "not found");
    m_TextureColor = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, imageColor->getFormat(), core::Dimension2D(imageColor->getDimension().width, imageColor->getDimension().height), imageColor->getMipMapsCount(), imageColor->getRawData(), "DiffuseColor");
    m_SamplerColor = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);

    resource::Image* imageNormal = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/bumpmap/bricks2_normal.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageNormal, "not found");
    m_TextureNormalmap = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, imageNormal->getFormat(), core::Dimension2D(imageNormal->getDimension().width, imageNormal->getDimension().height), imageNormal->getMipMapsCount(), imageNormal->getRawData(), "NormalMap");

    resource::Image* imageHeight = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/bumpmap/bricks2_disp.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
    ASSERT(imageHeight, "not found");
    m_TextureHeightmap = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, imageHeight->getFormat(), core::Dimension2D(imageHeight->getDimension().width, imageHeight->getDimension().height), imageHeight->getMipMapsCount(), imageHeight->getRawData(), "ParallaxMapping");
    
    m_SamplerFilter = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);

    std::vector<std::pair<std::string, std::string>> constants =
    {
        { "LIGHT_COUNT", std::to_string(countLights) },
        { "HEIGHT_SCALE", std::to_string(0.05f) },

        { "CONSTANT", std::to_string(1.0) },
        { "LINEAR", std::to_string(0.09) },
        { "QUADRATIC", std::to_string(0.032) }
    };

    //const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/parallaxmap.vert");
    //const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/parallaxmap.frag", constants);
    //std::vector<const renderer::Shader*> shaders = { vertShader, fragShader };
    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList.getContext(), "resources/bumpmap/parallaxmap.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_PS", renderer::ShaderType::Fragment }

        }, constants);

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

    resource::ResourceLoaderManager::getInstance()->remove(imageColor);
    resource::ResourceLoaderManager::getInstance()->remove(imageNormal);
    resource::ResourceLoaderManager::getInstance()->remove(imageHeight);
}

void ForwardParallaxMappingTest::Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights)
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

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
    }

    {
        struct LIGHT
        {
            core::Vector4D position;
            core::Vector4D diffuse;
            core::Vector4D specular;
        };

        std::vector<LIGHT> light(lights.size());
        for (u32 l = 0; l < lights.size(); ++l)
        {
            light[l].position = { std::get<0>(lights[l]), 1.0f };
            light[l].diffuse = std::get<1>(lights[l]);
            light[l].specular = core::Vector4D(1.0f);
        }

        struct UBO
        {
            core::Vector4D viewPosition;
        } ubo;

        ubo.viewPosition = { camera->getViewPosition(), 1.0 };

        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "light" }, 0, sizeof(LIGHT) * (u32)light.size(), light.data());
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "ps_ubo" }, 0, sizeof(UBO), &ubo);

        m_Program->bindSampler<renderer::ShaderType::Fragment>({ "samplerColor" }, m_SamplerColor.get());
        m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureColor" }, m_TextureColor.get());

        m_Program->bindSampler<renderer::ShaderType::Fragment>({ "samplerFilter" }, m_SamplerFilter.get());
        m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureNormal" }, m_TextureNormalmap.get());
        m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "textureHeight" }, m_TextureHeightmap.get());
    }

    geometry->draw(&m_CommandList);
}

void ForwardParallaxMappingTest::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    m_SamplerColor = nullptr;
    m_TextureColor = nullptr;

    m_SamplerFilter = nullptr;
    m_TextureNormalmap = nullptr;
    m_TextureHeightmap = nullptr;
}
} //namespace v3d