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

constexpr u32 k_maxDrawModeIndex = 3;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver()))
    , m_InputEventHandler(new InputEventHandler())

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_UnLit(nullptr)
    , m_Lambert(nullptr)
    , m_PhongTextureless(nullptr)

    , m_ArcballCameraHelper(nullptr)
    , m_FPSCameraHelper(nullptr)
{
    ASSERT(m_Window, "windows is nullptr");
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);

    m_Geometry.fill(nullptr);
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

    m_ArcballCameraHelper = new v3d::scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), 5.0f, 4.0f, 20.0f);
    m_ArcballCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.1f, 40.f);
    m_FPSCameraHelper = new v3d::scene::CameraFPSHelper(new v3d::scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, -10.0f));
    m_FPSCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.1f, 40.f);
    m_CameraHelper = m_ArcballCameraHelper;

    m_InputEventHandler->connect([this](const MouseInputEvent* event)
        {
            if (m_CameraHelper == m_ArcballCameraHelper)
            {
                m_ArcballCameraHelper->handlerMouseCallback(m_InputEventHandler, event);
            }
            else
            {
                m_FPSCameraHelper->rotateHandlerCallback(m_InputEventHandler, event, false);
            }
        });

    m_InputEventHandler->connect([this](const KeyboardInputEvent* event)
        {
            if (m_CameraHelper == m_ArcballCameraHelper)
            {
                if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
                {
                    if (event->_key == event::KeyCode::KeyKey_L)
                    {
                        m_LightIndex++;
                        if (m_LightIndex >= m_Lights.size())
                        {
                            m_LightIndex = 0;
                        }
                    }
                }

                f32 moveSpeed = 0.1f;
                v3d::core::Vector3D& lightPosition = std::get<0>(m_Lights[m_LightIndex]);
                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_W))
                {
                    lightPosition.y += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_S))
                {
                    lightPosition.y -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_A))
                {
                    lightPosition.x += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_D))
                {
                    lightPosition.x -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_Q))
                {
                    lightPosition.z += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_E))
                {
                    lightPosition.z -= moveSpeed;
                }
            }
            else
            {
                m_FPSCameraHelper->moveHandlerCallback(m_InputEventHandler, event);
            }

            {
                f32 roatateSpeed = 5.0f;
                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyLeft))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x, m_Transform.getRotation().y - roatateSpeed, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyRight))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x, m_Transform.getRotation().y + roatateSpeed, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyUp))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x + roatateSpeed, m_Transform.getRotation().y, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyDown))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x - roatateSpeed, m_Transform.getRotation().y, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_R))
                {
                    m_Transform.setRotation({ 0.0, 0.0, 0.0 });
                }
            }

            if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
            {
                if (event->_key == event::KeyCode::KeySpace)
                {
                    m_DrawMode++;
                    if (m_DrawMode > k_maxDrawModeIndex)
                    {
                        m_DrawMode = 0;
                    }
                }

                if (event->_key == event::KeyCode::KeyKey_G)
                {
                    m_GeometryIndex++;
                    if (m_GeometryIndex > 2)
                    {
                        m_GeometryIndex = 0;
                    }
                }

                if (event->_key == event::KeyCode::KeyKey_C)
                {
                    if (m_CameraHelper == m_ArcballCameraHelper)
                    {
                        m_CameraHelper = m_FPSCameraHelper;
                    }
                    else
                    {
                        m_CameraHelper = m_ArcballCameraHelper;
                    }
                }
            }

        });


    m_UnLit = new UnlitTextureTest(*m_CommandList);
    m_Lambert = new ForwardDirectionalLightTextureTest(*m_CommandList);
    m_PhongTextureless = new ForwardPointLightTest(*m_CommandList);

    Load();
}

void MyApplication::Load()
{
    m_Lights.push_back({ { 0.0, 0.0, -2.0 }, { 1.0, 1.0, 1.0, 1.0 } });
    m_Lights.push_back({ { 1.0, 1.0, -2.0 }, { 1.0, 0.0, 0.0, 1.0 } });
    m_Lights.push_back({ { -1.0, -1.0, -2.0 }, { 0.0, 0.0, 1.0, 1.0 } });
    m_Lights.push_back({ { -1.0, 1.0, -2.0 }, { 0.0, 1.0, 0.0, 1.0 } });
    m_Lights.push_back({ { 1.0, -1.0, -2.0 }, { 1.0, 1.0, 0.0, 1.0 } });

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

    {
        v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/cube.dae", resource::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
        m_Geometry[0] = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { cube });

        v3d::scene::Model* torus = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/torus.dae", resource::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
        m_Geometry[1] = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { torus });

        v3d::scene::Model* teapot = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/teapot.dae", resource::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
        m_Geometry[2] = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { teapot });

        m_Transform.setScale({ 100.0, 100.0, 100.0 });
    }

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
        m_LightDebug.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry.front()->getVertexInputAttribDescription(0, 0), m_LightDebug.m_Program.get(), m_RenderTarget.get());
        m_LightDebug.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_LightDebug.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_LightDebug.m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_LightDebug.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_LightDebug.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_LightDebug.m_Pipeline->setDepthWrite(true);
        m_LightDebug.m_Pipeline->setDepthTest(true);
    }

    m_UnLit->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0));
    m_Lambert->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
    m_PhongTextureless->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());

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

        std::vector<std::pair<std::string, std::string>> constants =
        {
            { "CONSTANT", std::to_string(1.0) },
            { "LINEAR", std::to_string(0.09) },
            { "QUADRATIC", std::to_string(0.032) }
        };

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/phong/pointLight.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/phong/pointLight.frag", constants);

        m_Phong.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_Phong.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry.front()->getVertexInputAttribDescription(0, 0), m_Phong.m_Program.get(), m_RenderTarget.get());
        m_Phong.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Phong.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_Phong.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Phong.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Phong.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Phong.m_Pipeline->setDepthWrite(true);
        m_Phong.m_Pipeline->setDepthTest(true);
    }

    {
        m_Normalmap.m_SamplerColor = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Normalmap.m_SamplerColor->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* imageColor = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/normalmap/brickwall.jpg");
        ASSERT(imageColor, "not found");
        m_Normalmap.m_TextureColor = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
            imageColor->getFormat(), core::Dimension2D(imageColor->getDimension().width, imageColor->getDimension().height), 1, 1, imageColor->getRawData(), "DiffuseColor");

        m_Normalmap.m_SamplerNormalmap = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);

        resource::Image* imageNormal = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/normalmap/brickwall_normal.jpg");
        ASSERT(imageNormal, "not found");
        m_Normalmap.m_TextureNormalmap = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
            imageNormal->getFormat(), core::Dimension2D(imageNormal->getDimension().width, imageNormal->getDimension().height), 1, 1, imageNormal->getRawData(), "NormalMap");

        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/normalmap/pointLight.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/normalmap/pointLight.frag");

        m_Normalmap.m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_Normalmap.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry.front()->getVertexInputAttribDescription(0, 0), m_Normalmap.m_Program.get(), m_RenderTarget.get());
        m_Normalmap.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Normalmap.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_Normalmap.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Normalmap.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Normalmap.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Normalmap.m_Pipeline->setDepthWrite(true);
        m_Normalmap.m_Pipeline->setDepthTest(true);
    }
}

bool MyApplication::Running()
{
    //Frame
    m_CommandList->beginFrame();

    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setRenderTarget(m_RenderTarget.get());

    Update();

    const bool axis = true;
    switch (m_DrawMode)
    {
    default:
    case 0:
        m_UnLit->Draw(m_Geometry[m_GeometryIndex], &m_CameraHelper->getCamera(), m_Transform);
        break;

    case 1:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_Lambert->Draw(m_Geometry[m_GeometryIndex], &m_CameraHelper->getCamera(), m_Transform, m_Lights);
        break;

    case 2:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_PhongTextureless->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform, m_Lights);
        break;

    case 3:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_Phong.Draw(m_CommandList, m_Geometry[m_GeometryIndex], m_CameraHelper, m_LightPosition, m_LightColor);
        break;

    case 4:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_Normalmap.Draw(m_CommandList, m_Geometry[m_GeometryIndex], m_CameraHelper, m_LightPosition, m_LightColor);
        break;

    case 5:
        break;
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

void MyApplication::Update()
{
    m_CameraHelper->update(0.0001f);
    m_Window->setTextCaption("View [" + std::to_string(m_CameraHelper->getViewPosition().x) + "; " + std::to_string(m_CameraHelper->getViewPosition().y) + "; " + std::to_string(m_CameraHelper->getViewPosition().z) + "]");
}

void MyApplication::Exit()
{
    {
        for (auto geometry : m_Geometry)
        {
            delete geometry;
            geometry = nullptr;
        }

        m_UnLit->Free();
        delete m_UnLit;
        m_UnLit = nullptr;

        m_Lambert->Free();
        delete m_Lambert;
        m_Lambert = nullptr;

        m_Phong.Free();
        m_Normalmap.Free();

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

    if (m_ArcballCameraHelper)
    {
        delete m_ArcballCameraHelper;
        m_ArcballCameraHelper = nullptr;
    }


    if (m_FPSCameraHelper)
    {
        delete m_FPSCameraHelper;
        m_FPSCameraHelper = nullptr;
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

void MyApplication::LightDebug::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::Camera* camera, const LightsProp& lights)
{
    commandList->setPipelineState(m_Pipeline.get());

    for (auto& light : lights)
    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D viewMatrix;
        } ubo;

        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setTranslation(std::get<0>(light));
        ubo.modelMatrix.setScale({ 10.0, 10.0, 10.0 });
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &std::get<1>(light));

        geometry->draw();
    }
}

void MyApplication::LightDebug::Free()
{
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


void MyApplication::NormalMap::Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor)
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
        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerColor" }, m_SamplerColor.get());
        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "samplerNormal" }, m_SamplerNormalmap.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureColor" }, m_TextureColor.get());
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "textureNormal" }, m_TextureNormalmap.get());
    }

    geometry->draw();
}

void MyApplication::NormalMap::Free()
{
    m_SamplerColor = nullptr;
    m_TextureColor = nullptr;
    m_SamplerNormalmap = nullptr;
    m_TextureNormalmap = nullptr;
    m_Pipeline = nullptr;
    m_Program = nullptr;
}
