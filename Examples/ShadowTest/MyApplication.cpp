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

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

#include "Stream/StreamManager.h"

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

    , m_FPSCameraHelper(nullptr)
    , m_Scene(nullptr)

    , m_ShadowMapping(nullptr)
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

    m_FPSCameraHelper = new v3d::scene::CameraFPSHelper(new v3d::scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), core::Vector3D(0.0f, 1.0f, -4.0f));
    m_FPSCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.01f, 256.f);

    m_InputEventHandler->connect([this](const MouseInputEvent* event)
        {
            m_FPSCameraHelper->rotateHandlerCallback(m_InputEventHandler, event, false);
        });

    m_InputEventHandler->connect([this](const KeyboardInputEvent* event)
        {
            f32 moveSpeed = 0.1f;
            v3d::core::Vector3D& lightPosition = m_LightPosition;
            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_I))
            {
                lightPosition.y += moveSpeed;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_K))
            {
                lightPosition.y -= moveSpeed;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_J))
            {
                lightPosition.x -= moveSpeed;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_L))
            {
                lightPosition.x += moveSpeed;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_U))
            {
                lightPosition.z -= moveSpeed;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_O))
            {
                lightPosition.z += moveSpeed;
            }


            m_FPSCameraHelper->moveHandlerCallback(m_InputEventHandler, event);
        });

    Load();
}

void MyApplication::Load()
{
    resource::ResourceLoaderManager::getInstance()->addPath("examples/shadowtest/");

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), "RenderTarget");
    m_RenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(), 
        { 
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) 
        }, 
        { 
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present 
        });
    renderer::Texture2D* depthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_Window->getSize(), renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
    m_RenderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_ShadowSampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_ShadowSampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);


    {
        v3d::scene::Model* scene = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/scene_t.dae");
        m_Scene = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { scene });
    }

    {
        m_ColorSampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_ColorSampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("resources/brickwall.jpg");
        ASSERT(image, "not found");
        m_ColorTexture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, 1, image->getRawData(), "ColorTexture");
    }

    {
        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "resources/solid.vert");
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "resources/solid.frag");

        m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
        m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Scene->getVertexInputAttribDescription(0, 0), m_Program.get(), m_RenderTarget.get());
        m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Pipeline->setDepthWrite(true);
        m_Pipeline->setDepthTest(true);

        m_CommandList->flushCommands();
    }

    m_LightDebug.Init(m_CommandList, m_RenderTarget.get());

    m_ShadowMapping = new ShadowMapping(m_CommandList);
    m_ShadowMapping->Init(m_Scene->getVertexInputAttribDescription(0, 0));
}

bool MyApplication::Running()
{
    Update(0.0001f);

    //Frame
    m_CommandList->beginFrame();

    m_ShadowMapping->Draw(m_Scene, m_Transform);

    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setRenderTarget(m_RenderTarget.get());

    {
        m_CommandList->setPipelineState(m_Pipeline.get());

        {
            struct UBO
            {
                core::Matrix4D projectionMatrix;
                core::Matrix4D modelMatrix;
                core::Matrix4D normalMatrix;
                core::Matrix4D viewMatrix;
                core::Matrix4D lightSpaceMatrix;
            } ubo;


            ubo.projectionMatrix = m_FPSCameraHelper->getProjectionMatrix();
            ubo.modelMatrix = m_Transform.getTransform();
            ubo.modelMatrix.getInverse(ubo.normalMatrix);
            ubo.normalMatrix.makeTransposed();
            ubo.viewMatrix = m_FPSCameraHelper->getViewMatrix();
            ubo.lightSpaceMatrix = m_ShadowMapping->GetLightSpaceMatrix();

            m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);
        }

        {
            struct UBO
            {
                core::Vector4D lightPosition;
                core::Vector4D viewPosition;
            } ubo;

            ubo.lightPosition = m_LightPosition;
            ubo.viewPosition = m_FPSCameraHelper->getPosition();

            m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);
            //m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "colorSampler" }, m_ColorSampler.get());
            //m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "colorTexture" }, m_ColorTexture.get());
            m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "shadowSampler" }, m_ShadowSampler.get());
            m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "shadowMap" }, m_ShadowMapping->GetDepthMap());
        }

        m_Scene->draw();
    }

    m_LightDebug.Draw(m_CommandList, &m_FPSCameraHelper->getCamera(), m_LightPosition);

    m_CommandList->endFrame();
    m_CommandList->presentFrame();
    
    m_CommandList->flushCommands();

    return true;
}

void MyApplication::Update(f32 dt)
{
    m_FPSCameraHelper->update(dt);
    m_ShadowMapping->Update(dt, m_LightPosition, core::Vector3D(0.0f, 0.0f, 0.0f));

    core::Matrix4D transform;
    transform.makeIdentity();
    //transform.setScale({0.05, 0.05, 0.05 });
   /* transform.setTranslation({ m_lightPosition .x, m_lightPosition.y, m_lightPosition.z});
    transform.setScale({40.0, 40.0, 40.0 });*/
    m_Transform.setTransform(transform);
    m_Window->setTextCaption("Target [" + std::to_string(transform.getTranslation().x) + "; " + std::to_string(transform.getTranslation().y) + "; " + std::to_string(transform.getTranslation().z) + "]");
}

void MyApplication::Exit()
{
    m_ShadowMapping->Free();
    delete m_ShadowMapping;

    m_LightDebug.Free();

    if (m_RenderTarget->getDepthStencilTexture())
    {
        renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture();
        delete depthAttachment;
    }

    m_ColorSampler = nullptr;
    m_ColorTexture = nullptr;
    m_ShadowSampler = nullptr;

    m_RenderTarget = nullptr;
    m_Pipeline = nullptr;
    m_Program = nullptr;

    resource::ResourceLoaderManager::getInstance()->clear();

    if (m_FPSCameraHelper)
    {
        delete m_FPSCameraHelper;
        m_FPSCameraHelper = nullptr;
    }

    if (m_Scene)
    {
        delete m_Scene;
        m_Scene = nullptr;
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


void MyApplication::LightDebug::Init(v3d::renderer::CommandList* commandList, v3d::renderer::RenderTargetState* renderTarget)
{
    v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("resources/cube.dae");
    m_Geometry = v3d::scene::ModelHelper::createModelHelper(*commandList, { cube });

    renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList->getContext(), "resources/debug.vert");
    renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList->getContext(), "resources/debug.frag");

    m_Program = commandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = commandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
}

void MyApplication::LightDebug::Draw(v3d::renderer::CommandList* commandList, v3d::scene::Camera* camera, const v3d::core::Vector3D& light)
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
    ubo.modelMatrix.setTranslation(light);
    ubo.viewMatrix = camera->getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, sizeof(UBO), &ubo);
    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Fragment>({ "debugColor" }, 0, sizeof(core::Vector4D), &m_lightColor);

    m_Geometry->draw();
}

void MyApplication::LightDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    delete m_Geometry;
    m_Geometry = nullptr;
}