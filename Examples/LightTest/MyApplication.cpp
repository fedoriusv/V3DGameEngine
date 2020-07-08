#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Formats.h"

#include "Resource/Image.h"
#include "Scene/Model.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ModelFileLoader.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::resource;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver()))
    , m_InputEventHandler(new InputEventHandler())

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Geometry(nullptr)
{
    ASSERT(m_Window, "windows is nullptr");
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    while (true)
    {
        if (!Window::updateWindow(m_Window))
        {
            break;
        }

        if (m_Window->isValid())
        {
            m_Window->getInputEventReceiver()->sendDeferredEvents();
            MyApplication::Running();
        }
    }

    Exit();
    delete this;

    return 0;
}

void MyApplication::Initialize()
{
    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::ImmediateCommandList);

    m_CameraHelper = new v3d::scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), 5.0f, 4.0f, 20.0f);
    m_CameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.1f, 40.f);

    m_InputEventHandler->connect([this](const MouseInputEvent* event)
        {
            m_CameraHelper->handlerMouseCallback(m_InputEventHandler, event);
        });

    m_InputEventHandler->connect([this](const KeyboardInputEvent* event)
        {
            {
                f32 moveSpeed = 0.1f;
                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_W))
                {
                    m_LightPosition.y += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_S))
                {
                    m_LightPosition.y -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_A))
                {
                    m_LightPosition.x += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_D))
                {
                    m_LightPosition.x -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_Q))
                {
                    m_LightPosition.z += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_E))
                {
                    m_LightPosition.z -= moveSpeed;
                }
            }

            if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
            {
                if (event->_key == event::KeyCode::KeySpace)
                {
                    m_DrawMode++;
                    if (m_DrawMode > 2)
                    {
                        m_DrawMode = 0;
                    }
                }
            }

        });

    Load();
}

void MyApplication::Load()
{
    resource::ResourceLoaderManager::getInstance()->addPath("examples/lighttest/");

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), "RenderTarget");
    m_RenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(), 
        { 
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) 
        }, 
        { 
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present 
        });
    renderer::Texture2D* depthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_Window->getSize(), renderer::TextureSamples::TextureSamples_x1);
    m_RenderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/cube1.dae");
    m_Geometry = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { cube });

    {
        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debugAxis.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.frag");

        m_AxisDebug.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_AxisDebug.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_AxisDebug.m_Program.get(), m_RenderTarget.get());
        m_AxisDebug.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
        m_AxisDebug.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_AxisDebug.m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_AxisDebug.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_AxisDebug.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        m_AxisDebug.m_Pipeline->setDepthWrite(true);
        m_AxisDebug.m_Pipeline->setDepthTest(true);
    }

    {
        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.frag");

        m_LightDebug.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_LightDebug.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_LightDebug.m_Program.get(), m_RenderTarget.get());
        m_LightDebug.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_LightDebug.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_LightDebug.m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_LightDebug.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_LightDebug.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_LightDebug.m_Pipeline->setDepthWrite(true);
        m_LightDebug.m_Pipeline->setDepthTest(true);
    }

    {
        m_UnLit.m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
        m_UnLit.m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/unlit/basetex.jpg");
        ASSERT(image, "not found");
        m_UnLit.m_Texture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, 1, image->getRawData(), "UnlitTexture");

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/unlit/texture.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/unlit/texture.frag");

        m_UnLit.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_UnLit.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_UnLit.m_Program.get(), m_RenderTarget.get());
        m_UnLit.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_UnLit.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_UnLit.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_UnLit.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_UnLit.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_UnLit.m_Pipeline->setDepthWrite(true);
        m_UnLit.m_Pipeline->setDepthTest(true);
    }

    {
        m_Lambert.m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Lambert.m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/lambert/box.jpg");
        ASSERT(image, "not found");
        m_Lambert.m_Texture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, 1, image->getRawData(), "LambertTexture");

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/lambert/pointLight.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/lambert/pointLight.frag");

        m_Lambert.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_Lambert.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Lambert.m_Program.get(), m_RenderTarget.get());
        m_Lambert.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Lambert.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_Lambert.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Lambert.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Lambert.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Lambert.m_Pipeline->setDepthWrite(true);
        m_Lambert.m_Pipeline->setDepthTest(true);
    }

    {
        m_Phong.m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Phong.m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* imageDiffuse = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/phong/container2.png");
        ASSERT(imageDiffuse, "not found");
        m_Phong.m_TextureDiffuse = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
            imageDiffuse->getFormat(), core::Dimension2D(imageDiffuse->getDimension().width, imageDiffuse->getDimension().height), 1, 1, imageDiffuse->getRawData(), "PhongDiffuse");

        resource::Image* imageSpecular = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/phong/container2_specular.png");
        ASSERT(imageSpecular, "not found");
        m_Phong.m_TextureSpecular = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
            imageSpecular->getFormat(), core::Dimension2D(imageSpecular->getDimension().width, imageSpecular->getDimension().height), 1, 1, imageSpecular->getRawData(), "PhongSpecular");

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/phong/pointLight.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/phong/pointLight.frag");

        m_Phong.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_Phong.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Phong.m_Program.get(), m_RenderTarget.get());
        m_Phong.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Phong.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_Phong.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Phong.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Phong.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Phong.m_Pipeline->setDepthWrite(true);
        m_Phong.m_Pipeline->setDepthTest(true);
    }
}

bool MyApplication::Running()
{
    //Frame
    m_CommandList->beginFrame();

    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setRenderTarget(m_RenderTarget.get());

    m_CameraHelper->update(0);

    const bool axis = true;
    switch (m_DrawMode)
    {
    default:
    case 0:
        m_UnLit.Draw(m_CommandList, m_Geometry, &m_CameraHelper->getCamera());
        break;

    case 1:
        m_LightDebug.Draw(m_CommandList, m_Geometry, &m_CameraHelper->getCamera(), m_LightPosition, m_LightColor);
        m_Lambert.Draw(m_CommandList, m_Geometry, &m_CameraHelper->getCamera(), m_LightPosition, m_LightColor);
        break;

    case 2:
        m_LightDebug.Draw(m_CommandList, m_Geometry, &m_CameraHelper->getCamera(), m_LightPosition, m_LightColor);
        m_Phong.Draw(m_CommandList, m_Geometry, m_CameraHelper, m_LightPosition, m_LightColor);
        break;

    //case 3:
        //bumpmap/parallax
    }
    
    if (axis)
    {
        m_AxisDebug.Draw(m_CommandList, &m_CameraHelper->getCamera());
    }

    m_CommandList->endFrame();
    m_CommandList->presentFrame();
    
    m_CommandList->flushCommands();

    return true;
}

void MyApplication::Exit()
{
    {
        if (m_Geometry)
        {
            delete m_Geometry;
            m_Geometry = nullptr;
        }

        m_UnLit.Free();
        m_Lambert.Free();
        m_Phong.Free();

        m_LightDebug.Free();
        m_AxisDebug.Free();
    }
    if (m_RenderTarget->getDepthStencilTexture())
    {
        renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture();
        delete depthAttachment;
    }
    m_RenderTarget = nullptr;

    resource::ResourceLoaderManager::getInstance()->clear();

    if (m_CameraHelper)
    {
        delete m_CameraHelper;
        m_CameraHelper = nullptr;
    }

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent);
    if (m_InputEventHandler)
    {
        delete m_InputEventHandler;
        m_InputEventHandler = nullptr;
    }

    if (m_CommandList)
    {
        delete m_CommandList;
        m_CommandList = nullptr;
    }

    if (m_Context)
    {
        renderer::Context::destroyContext(m_Context);
        m_Context = nullptr;
    }
}

MyApplication::~MyApplication()
{
    Window::detroyWindow(m_Window);
}

void MyApplication::AxisDebug::Draw(v3d::renderer::CommandList* commandList, v3d::scene::Camera* camera)
{
    commandList->setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    ubo.modelMatrix.makeIdentity();
    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.setRotation({ 0.0, 0.0, 0.0 });
        ubo.viewMatrix = camera->getViewMatrix();

        const core::Vector4D red = { 1.0, 0.0, 0.0, 1.0 };
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &red);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.setRotation({0.0, 0.0, 90.0});
        ubo.viewMatrix = camera->getViewMatrix();

        const core::Vector4D green = { 0.0, 1.0, 0.0, 1.0 };
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &green);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.setRotation({ 0.0, 90.0, 0.0 });
        ubo.viewMatrix = camera->getViewMatrix();

        const core::Vector4D blue = { 0.0, 0.0, 1.0, 1.0 };
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &blue);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 2, 1);
    }

}

void MyApplication::AxisDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;
}

void MyApplication::LightDebug::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::Camera* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor)
{
    commandList->setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    ubo.projectionMatrix = camera->getProjectionMatrix();
    ubo.modelMatrix.makeIdentity();
    ubo.modelMatrix.setTranslation(lightPosition);
    ubo.modelMatrix.setScale({ 10.0, 10.0, 10.0 });
    ubo.viewMatrix = camera->getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &lightColor);

    geometry->draw();
}

void MyApplication::LightDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;
}

void MyApplication::UnLit::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::Camera* camera)
{
    commandList->setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    ubo.projectionMatrix = camera->getProjectionMatrix();
    ubo.modelMatrix.makeIdentity();
    ubo.modelMatrix.setScale({ 100.0, 100.0, 100.0 });
    ubo.viewMatrix = camera->getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);
    m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_Sampler.get());
    m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureColor" }, m_Texture.get());

    geometry->draw();
}

void MyApplication::UnLit::Free()
{
    m_Sampler = nullptr;
    m_Texture = nullptr;
    m_Pipeline = nullptr;
    m_Program = nullptr;
}


void MyApplication::Lambert::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::Camera* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor)
{
    commandList->setPipelineState(m_Pipeline.get());

    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D normalMatrix;
            core::Matrix4D viewMatrix;
        } ubo;

        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setScale({ 100.0, 100.0, 100.0 });
        ubo.modelMatrix.getInverse(ubo.normalMatrix);
        ubo.normalMatrix.makeTransposed();
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
    }

    {
        struct UBO
        {
            core::Vector4D lightPosition;
            core::Vector4D lightColor;
        } light;

        light.lightPosition = { lightPosition, 1.0 };
        light.lightColor = lightColor;

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "light" }, 0, sizeof(UBO), &light);
        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_Sampler.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureColor" }, m_Texture.get());
    }

    geometry->draw();
}

void MyApplication::Lambert::Free()
{
    m_Sampler = nullptr;
    m_Texture = nullptr;
    m_Pipeline = nullptr;
    m_Program = nullptr;
}


void MyApplication::Phong::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor)
{
    commandList->setPipelineState(m_Pipeline.get());

    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D normalMatrix;
            core::Matrix4D viewMatrix;
        } ubo;

        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setScale({ 100.0, 100.0, 100.0 });
        ubo.modelMatrix.getInverse(ubo.normalMatrix);
        ubo.normalMatrix.makeTransposed();
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
    }

    {
        struct UBO
        {
            core::Vector4D viewPosition;
            core::Vector4D lightPosition;
            core::Vector4D lightColor;
        } ubo;

        ubo.lightColor = lightColor;
        ubo.lightPosition = { lightPosition, 1.0 };
        ubo.viewPosition = { camera->getPosition(), 1.0 };

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_Sampler.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureDiffuse" }, m_TextureDiffuse.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureSpecular" }, m_TextureSpecular.get());
    }

    geometry->draw();
}

void MyApplication::Phong::Free()
{
    m_Sampler = nullptr;
    m_TextureDiffuse = nullptr;
    m_TextureSpecular = nullptr;
    m_Pipeline = nullptr;
    m_Program = nullptr;
}


