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

constexpr u32 k_maxDrawModeIndex = 5; //unlit = 0, directional = 1, phong = 2, phongTexture = 3, normal = 4, parallax = 5
constexpr u32 k_countGeometryModels = 3; //cube , torus, teapot

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver()))
    , m_InputEventHandler(new InputEventHandler())

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_ArcballCameraHelper(nullptr)
    , m_FPSCameraHelper(nullptr)

    , m_UnLit(nullptr)
    , m_DirectionalLight(nullptr)
    , m_PointLightTextureless(nullptr)
    , m_PointLight(nullptr)
    , m_NormalMap(nullptr)
    , m_ParallaxMap(nullptr)
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
    m_ArcballCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.1f, 30.f);
    m_FPSCameraHelper = new v3d::scene::CameraFPSHelper(new v3d::scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, -5.0f));
    m_FPSCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.1f, 30.f);
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
                    lightPosition.x -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_D))
                {
                    lightPosition.x += moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_Q))
                {
                    lightPosition.z -= moveSpeed;
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_E))
                {
                    lightPosition.z += moveSpeed;
                }
            }
            else
            {
                m_FPSCameraHelper->moveHandlerCallback(m_InputEventHandler, event);
            }

            {
                f32 rotateSpeed = 5.0f;
                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyLeft))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x, m_Transform.getRotation().y - rotateSpeed, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyRight))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x, m_Transform.getRotation().y + rotateSpeed, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyUp))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x - rotateSpeed, m_Transform.getRotation().y, m_Transform.getRotation().z });
                }

                if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyDown))
                {
                    m_Transform.setRotation({ m_Transform.getRotation().x + rotateSpeed, m_Transform.getRotation().y, m_Transform.getRotation().z });
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
                    if (m_GeometryIndex >= k_countGeometryModels)
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
    m_DirectionalLight = new ForwardDirectionalLightTextureTest(*m_CommandList);
    m_PointLightTextureless = new ForwardPointLightTest(*m_CommandList);
    m_PointLight = new ForwardPointLightTextureTest(*m_CommandList);
    m_NormalMap = new ForwardNormalMapTest(*m_CommandList);
    m_ParallaxMap = new ForwardParallaxMappingTest(*m_CommandList);

    Load();

    m_CommandList->submitCommands(true);
    m_CommandList->flushCommands();
}

void MyApplication::Load()
{
    m_Lights.push_back({ { 0.0, 0.0, -2.0 }, { 1.0, 1.0, 1.0, 1.0 } });
    m_Lights.push_back({ { 1.0, 1.0, -2.0 }, { 1.0, 0.0, 0.0, 1.0 } });
    m_Lights.push_back({ { -1.0, -1.0, -2.0 }, { 0.0, 0.0, 1.0, 1.0 } });
    m_Lights.push_back({ { -1.0, 1.0, -2.0 }, { 0.0, 1.0, 0.0, 1.0 } });
    m_Lights.push_back({ { 1.0, -1.0, -2.0 }, { 1.0, 1.0, 0.0, 1.0 } });

    resource::ResourceLoaderManager::getInstance()->addPath("examples/lighttest/");

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), 0, "RenderTarget");
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
        m_Geometry[0] = v3d::scene::ModelHelper::createModelHelper(m_CommandList, { cube });

        v3d::scene::Model* torus = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/torus.dae", resource::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
        m_Geometry[1] = v3d::scene::ModelHelper::createModelHelper(m_CommandList, { torus });

        v3d::scene::Model* teapot = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/teapot.dae", resource::ModelLoaderFlag_GenerateTangentAndBitangent | resource::ModelLoaderFlag::ModelLoaderFlag_UseBitangent);
        m_Geometry[2] = v3d::scene::ModelHelper::createModelHelper(m_CommandList, { teapot });

        m_Transform.setScale({ 100.0f, 100.0f, 100.0f });
    }

    {
        //const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debugAxis.vert");
        //const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.frag");
        //std::vector<const renderer::Shader*> shaders = { vertShader, fragShader };
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debugAxis.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_PS", renderer::ShaderType::Fragment }

            });

        m_AxisDebug.m_Program = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_AxisDebug.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_AxisDebug.m_Program.get(), m_RenderTarget.get());
        m_AxisDebug.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
        m_AxisDebug.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_AxisDebug.m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_AxisDebug.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_AxisDebug.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        m_AxisDebug.m_Pipeline->setDepthWrite(true);
        m_AxisDebug.m_Pipeline->setDepthTest(true);
    }

    {
        //const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.vert");
        //const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.frag");
        //std::vector<const renderer::Shader*> shaders = { vertShader, fragShader };
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "resources/debug/debug.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_PS", renderer::ShaderType::Fragment }

            });

        m_LightDebug.m_Program = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_LightDebug.m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry.front()->getVertexInputAttribDescription(0, 0), m_LightDebug.m_Program.get(), m_RenderTarget.get());
        m_LightDebug.m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_LightDebug.m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_LightDebug.m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_LightDebug.m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_LightDebug.m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_LightDebug.m_Pipeline->setDepthWrite(true);
        m_LightDebug.m_Pipeline->setDepthTest(true);
    }

    m_UnLit->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0));
    m_DirectionalLight->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
    m_PointLightTextureless->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
    m_PointLight->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
    m_NormalMap->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
    m_ParallaxMap->Load(m_RenderTarget.get(), m_Geometry.front()->getVertexInputAttribDescription(0, 0), (u32)m_Lights.size());
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
        m_UnLit->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform);
        break;

    case 1:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_DirectionalLight->Draw(m_Geometry[m_GeometryIndex], &m_CameraHelper->getCamera(), m_Transform, m_Lights);
        break;

    case 2:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_PointLightTextureless->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform, m_Lights);
        break;

    case 3:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_PointLight->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform, m_Lights);
        break;

    case 4:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_NormalMap->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform, m_Lights);
        break;

    case 5:
        m_LightDebug.Draw(m_CommandList, m_Geometry[0], &m_CameraHelper->getCamera(), m_Lights);
        m_ParallaxMap->Draw(m_Geometry[m_GeometryIndex], m_CameraHelper, m_Transform, m_Lights);
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
    m_Window->setTextCaption("Target [" + std::to_string(m_CameraHelper->getViewPosition().x) + "; " + std::to_string(m_CameraHelper->getViewPosition().y) + "; " + std::to_string(m_CameraHelper->getViewPosition().z) + "]");
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

        m_DirectionalLight->Free();
        delete m_DirectionalLight;
        m_DirectionalLight = nullptr;

        m_PointLightTextureless->Free();
        delete m_PointLightTextureless;
        m_PointLightTextureless = nullptr;

        m_PointLight->Free();
        delete m_PointLight;
        m_PointLight = nullptr;

        m_NormalMap->Free();
        delete m_NormalMap;
        m_NormalMap = nullptr;

        m_ParallaxMap->Free();
        delete m_ParallaxMap;
        m_ParallaxMap = nullptr;

        m_LightDebug.Free();
        m_AxisDebug.Free();
    }
    if (m_RenderTarget->hasDepthStencilTexture())
    {
        renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture<renderer::Texture2D>();
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
        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &red);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.setRotation({0.0, 0.0, 90.0});
        ubo.viewMatrix = camera->getViewMatrix();

        const core::Vector4D green = { 0.0, 1.0, 0.0, 1.0 };
        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &green);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.setRotation({ 0.0, -90.0, 0.0 });
        ubo.viewMatrix = camera->getViewMatrix();

        const core::Vector4D blue = { 0.0, 0.0, 1.0, 1.0 };
        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &blue);

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

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_ubo" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &std::get<1>(light));

        geometry->draw(commandList);
    }
}

void MyApplication::LightDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;
}
