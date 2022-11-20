#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Core/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Texture.h"
#include "Renderer/RenderTargetState.h"
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
    , m_CascadeShadowMapping(nullptr)
    , m_ShadowMappingPoint(nullptr)

    , m_Mode(PointLight)
    , m_PercentageCloserFiltering(true)
    , m_Debug(false)
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
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

    m_FPSCameraHelper = new v3d::scene::CameraFPSHelper(new v3d::scene::Camera(core::Vector3D(0.0f, 0.0f, -8.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), core::Vector3D(0.0f, 1.0f, -4.0f));
    m_FPSCameraHelper->setPerspective(45.0f, m_Window->getSize(), 0.01f, 256.f);

    m_InputEventHandler->connect([this](const MouseInputEvent* event)
        {
            m_FPSCameraHelper->rotateHandlerCallback(m_InputEventHandler, event, false);
        });

    m_InputEventHandler->connect([this](const KeyboardInputEvent* event)
        {
            f32 moveSpeed = 0.1f;
            v3d::core::Vector3D& lightPosition = m_SunDirection;
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


            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_B))
            {
                m_Debug = !m_Debug;
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_M))
            {
                m_Mode = (ShadowMode)(m_Mode + 1);
                if (m_Mode == ShadowMode::MaxMode)
                {
                    m_Mode = ShadowMode::FirstMode;
                }
            }

            if (m_InputEventHandler->isKeyPressed(event::KeyCode::KeyKey_P))
            {
                m_PercentageCloserFiltering = !m_PercentageCloserFiltering;
            }
            
            m_FPSCameraHelper->moveHandlerCallback(m_InputEventHandler, event);
        });

    Load();
}

void MyApplication::Load()
{
    resource::ResourceLoaderManager::getInstance()->addPath("examples/shadowtest/");

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), 0, "RenderTarget");
    m_RenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(), 
        { 
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) 
        }, 
        { 
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present 
        });
    renderer::Texture2D* depthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_Window->getSize(), renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
    m_RenderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    m_ShadowSampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_ShadowSampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);
    m_ShadowSampler->setEnableCompareOp(true);
    m_ShadowSampler->setCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_ShadowSampler->setBorderColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    {
        v3d::scene::Model* scene = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("data/big_field.dae");
        m_Scene = v3d::scene::ModelHelper::createModelHelper(m_CommandList, { scene });
    }

    m_ShadowMapping = new ShadowMapping(m_CommandList);
    m_ShadowMapping->Init(m_Scene->getVertexInputAttribDescription(0, 0));

    m_CascadeShadowMapping = new CascadedShadowMapping(m_CommandList);
    m_CascadeShadowMapping->Init(m_Scene->getVertexInputAttribDescription(0, 0));

    m_ShadowMappingPoint = new ShadowMappingPoint(m_CommandList);
    m_ShadowMappingPoint->Init(m_Scene->getVertexInputAttribDescription(0, 0));

    m_LightDebug.Init(m_CommandList, m_RenderTarget.get());

    {
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/solid_SunShadow.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_FS", renderer::ShaderType::Fragment }
            });

        m_ShadowMappingProgram = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_ShadowMappingPipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Scene->getVertexInputAttribDescription(0, 0), m_ShadowMappingProgram.get(), m_RenderTarget.get());
        m_ShadowMappingPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_ShadowMappingPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_ShadowMappingPipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_ShadowMappingPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_ShadowMappingPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_ShadowMappingPipeline->setDepthWrite(true);
        m_ShadowMappingPipeline->setDepthTest(true);
    }

    {
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/solid_SunCascadedShadow.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_FS", renderer::ShaderType::Fragment }
            }, 
            { 
                { "SHADOWMAP_CASCADE_COUNT", std::to_string(CascadedShadowMapping::s_CascadeCount) },
                { "SHADER_DEBUG", std::to_string(0) }
            });

        m_CascadeShadowMappingProgram = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_CascadeShadowMappingPipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_ShadowMappingPipeline->getGraphicsPipelineStateDesc(), m_CascadeShadowMappingProgram.get(), m_RenderTarget.get());


        std::vector<const renderer::Shader*> shadersDebug = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/solid_SunCascadedShadow.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_FS", renderer::ShaderType::Fragment }
            },
            {
                { "SHADOWMAP_CASCADE_COUNT", std::to_string(CascadedShadowMapping::s_CascadeCount) },
                { "SHADER_DEBUG", std::to_string(1) }
            });

        m_CascadeShadowMappingProgramDebug = m_CommandList->createObject<renderer::ShaderProgram>(shadersDebug);
        m_CascadeShadowMappingPipelineDebug = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_ShadowMappingPipeline->getGraphicsPipelineStateDesc(), m_CascadeShadowMappingProgramDebug.get(), m_RenderTarget.get());
    }

    {
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/solid_ShadowPointLight.hlsl",
            {
                {"main_VS", renderer::ShaderType::Vertex },
                {"main_FS", renderer::ShaderType::Fragment }
            },
            {
                { "NEAR_PLANE", std::to_string(m_ShadowMappingPoint->GetCamera().getNear()) },
                { "FAR_PLANE", std::to_string(m_ShadowMappingPoint->GetCamera().getFar()) }
            });

        m_ShadowMappingPointProgram = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_ShadowMappingPointPipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_ShadowMappingPipeline->getGraphicsPipelineStateDesc(), m_ShadowMappingPointProgram.get(), m_RenderTarget.get());
    }

    m_CommandList->flushCommands();
}

void MyApplication::DrawDirectionLightMode(bool enablePCF, bool cascaded)
{
    if (cascaded)
    {
        m_CascadeShadowMapping->Draw(m_Scene, m_Transform);

        m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setRenderTarget(m_RenderTarget.get());

        m_CommandList->setPipelineState(m_Debug ? m_CascadeShadowMappingPipelineDebug.get() : m_CascadeShadowMappingPipeline.get());
        v3d::renderer::ShaderProgram* const cascadeShadowMappingProgram = m_Debug ? m_CascadeShadowMappingProgramDebug.get() : m_CascadeShadowMappingProgram.get();
        {
            {
                struct UBO
                {
                    core::Matrix4D projectionMatrix;
                    core::Matrix4D modelMatrix;
                    core::Matrix4D normalMatrix;
                    core::Matrix4D viewMatrix;
                    core::Matrix4D lightSpaceMatrix[v3d::CascadedShadowMapping::s_CascadeCount];
                } ubo;

                ubo.projectionMatrix = m_FPSCameraHelper->getProjectionMatrix();
                ubo.modelMatrix = m_Transform.getTransform();
                ubo.modelMatrix.getInverse(ubo.normalMatrix);
                ubo.normalMatrix.makeTransposed();
                ubo.viewMatrix = m_FPSCameraHelper->getViewMatrix();
                for (u32 i = 0; i < v3d::CascadedShadowMapping::s_CascadeCount; ++i)
                {
                    ubo.lightSpaceMatrix[i] = m_CascadeShadowMapping->GetLightSpaceMatrix()[i];
                }

                cascadeShadowMappingProgram->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);
            }

            {
                struct UBO
                {
                    core::Vector4D lightDirection;
                    core::Vector4D viewPosition;
                    core::Vector4D casadeSplits[CascadedShadowMapping::s_CascadeCount];
                } ubo;

                ubo.lightDirection = m_SunDirection;
                ubo.lightDirection.normalize();
                ubo.viewPosition = m_FPSCameraHelper->getPosition();
                for (u32 i = 0; i < v3d::CascadedShadowMapping::s_CascadeCount; ++i)
                {
                    ubo.casadeSplits[i].x = m_CascadeShadowMapping->GetCascadeSplits()[i];
                }

                cascadeShadowMappingProgram->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "fs_buffer" }, 0, (u32)sizeof(UBO), &ubo);
            }

            cascadeShadowMappingProgram->bindSampler<renderer::ShaderType::Fragment>({ "shadowSampler" }, m_ShadowSampler.get());
            cascadeShadowMappingProgram->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2DArray>({ "cascadedShadowMap" }, m_CascadeShadowMapping->GetDepthMap());
        }
    }
    else
    {
        m_ShadowMapping->Draw(m_Scene, m_Transform);

        m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setRenderTarget(m_RenderTarget.get());

        m_CommandList->setPipelineState(m_ShadowMappingPipeline.get());
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

            m_ShadowMappingProgram->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);
        }

        {
            struct UBO
            {
                core::Vector4D lightDirection;
                core::Vector4D viewPosition;
                u32 enablePCF;
            } ubo;

            ubo.lightDirection = m_SunDirection;
            ubo.lightDirection.normalize();
            ubo.viewPosition = m_FPSCameraHelper->getPosition();
            ubo.enablePCF = (u32)enablePCF;

            m_ShadowMappingProgram->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "fs_buffer" }, 0, (u32)sizeof(UBO), &ubo);
            m_ShadowMappingProgram->bindSampler<renderer::ShaderType::Fragment>({ "shadowSampler" }, m_ShadowSampler.get());
            m_ShadowMappingProgram->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "shadowMap" }, m_ShadowMapping->GetDepthMap());
        }
    }

    m_Scene->draw(m_CommandList);
}

void MyApplication::DrawPointLightMode(bool enablePCF)
{
    m_ShadowMappingPoint->Draw(m_Scene, m_Transform);

    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setRenderTarget(m_RenderTarget.get());
    m_CommandList->setPipelineState(m_ShadowMappingPointPipeline.get());

    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D normalMatrix;
            core::Matrix4D viewMatrix;
        } ubo;


        ubo.projectionMatrix = m_FPSCameraHelper->getProjectionMatrix();
        ubo.modelMatrix = m_Transform.getTransform();
        ubo.modelMatrix.getInverse(ubo.normalMatrix);
        ubo.normalMatrix.makeTransposed();
        ubo.viewMatrix = m_FPSCameraHelper->getViewMatrix();

        m_ShadowMappingPointProgram->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);
    }

    {
        struct UBO
        {
            core::Vector4D lightPosition;
            core::Vector4D viewPosition;
            u32 enablePCF;
        } ubo;

        ubo.lightPosition = m_SunDirection;
        ubo.viewPosition = m_FPSCameraHelper->getPosition();
        ubo.enablePCF = (u32)enablePCF;

        m_ShadowMappingPointProgram->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "fs_buffer" }, 0, (u32)sizeof(UBO), &ubo);

        m_ShadowMappingPointProgram->bindSampler<renderer::ShaderType::Fragment>({ "shadowSampler" }, m_ShadowSampler.get());
        m_ShadowMappingPointProgram->bindTexture<renderer::ShaderType::Fragment, renderer::TextureCube>({ "shadowMapCube" }, m_ShadowMappingPoint->GetDepthMap());
    }

    m_Scene->draw(m_CommandList);
}

bool MyApplication::Running()
{
    Update(0.0001f);

    //Frame
    m_CommandList->beginFrame();

    if (m_Mode == DirectionLight  || m_Mode == DirectionLightCascadeShadowing)
    {
        MyApplication::DrawDirectionLightMode(m_PercentageCloserFiltering, m_Mode == DirectionLightCascadeShadowing);
    }
    else if (m_Mode == PointLight)
    {
        MyApplication::DrawPointLightMode(m_PercentageCloserFiltering);
    }

    if (m_Debug)
    {
        m_LightDebug.Draw(m_CommandList, &m_FPSCameraHelper->getCamera(), m_SunDirection);
    }

    m_CommandList->endFrame();
    m_CommandList->presentFrame();
    
    m_CommandList->flushCommands();

    return true;
}

void MyApplication::Update(f32 dt)
{
    m_FPSCameraHelper->update(dt);
    if (m_Mode == DirectionLight)
    {
        /*
        const f32 k_sunConstant = 5.0f;
        const f32 k_sunKoeff = m_SunDirection.z > 0.f ? 1.0f : -1.0f;
        v3d::core::Vector3D testSunOffest(0, 0, k_sunConstant * k_sunKoeff);

        v3d::scene::Transform testSunPivot;
        testSunPivot.setPosition(m_FPSCameraHelper->getPosition() + testSunOffest);
        testSunPivot.setRotation(m_FPSCameraHelper->getRotation());

        v3d::scene::Transform testSunPosition;
        testSunPosition.setPosition(testSunOffest * k_sunKoeff);
        testSunPosition.setTransform(testSunPivot.getTransform() * testSunPosition.getTransform());
        testSunPosition.setPosition(core::Vector3D(testSunPosition.getPosition().x, m_SunDirection.y, testSunPosition.getPosition().z));

        v3d::core::Vector3D lightTarget;
        lightTarget = testSunPosition.getPosition() - m_SunDirection;
        lightTarget.y = 0.0f;

        m_ShadowMapping->Update(dt, testSunPosition.getPosition(), lightTarget);
        */
        m_ShadowMapping->Update(dt, m_SunDirection, {0,0,0});
    }
    else if (m_Mode == DirectionLightCascadeShadowing)
    {
        m_CascadeShadowMapping->Update(dt, m_FPSCameraHelper->getCamera(), m_SunDirection, { 0,0,0 });
    }
    else if (m_Mode == PointLight)
    {
        m_ShadowMappingPoint->Update(dt, m_SunDirection);
    }

    core::Matrix4D transform;
    transform.makeIdentity();
    //transform.setScale({0.05, 0.05, 0.05 });
    m_Transform.setTransform(transform);
}

void MyApplication::Exit()
{
    m_LightDebug.Free();

    if (m_RenderTarget->hasDepthStencilTexture())
    {
        renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture<renderer::Texture2D>();
        delete depthAttachment;
    }
    m_RenderTarget = nullptr;

    m_ShadowSampler = nullptr;


    m_ShadowMapping->Free();
    delete m_ShadowMapping;
    m_ShadowMappingPipeline = nullptr;
    m_ShadowMappingProgram = nullptr;

    m_CascadeShadowMapping->Free();
    delete m_CascadeShadowMapping;
    m_CascadeShadowMappingPipeline = nullptr;
    m_CascadeShadowMappingProgram = nullptr;
    m_CascadeShadowMappingPipelineDebug = nullptr;
    m_CascadeShadowMappingProgramDebug = nullptr;

    m_ShadowMappingPoint->Free();
    delete m_ShadowMappingPoint;
    m_ShadowMappingPointPipeline = nullptr;
    m_ShadowMappingPointProgram = nullptr;

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