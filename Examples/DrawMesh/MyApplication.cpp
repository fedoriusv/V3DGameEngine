#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Formats.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ShaderBinaryFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

#include "Scene/Model.h"
#include "Scene/Camera.h"

#include "Stream/StreamManager.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::resource;
using namespace v3d::scene;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Camera(nullptr)

    , m_Render(nullptr)
{
    core::Dimension2D widowsSize = { 1280, 720 };
    m_Window = Window::createWindow(widowsSize, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_Camera = new CameraArcballHelper(new Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), 5.0f);
    m_Camera->setPerspective(45.0f, widowsSize, 1.f, 5.f);

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect(std::bind(&CameraArcballHelper::handlerMouseCallback, m_Camera, m_InputEventHandler, std::placeholders::_1));

    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    bool running = true;
    while (running)
    {
        running = Window::updateWindow(m_Window);

        m_Window->getInputEventReceiver()->sendDeferredEvents();
        if (running)
        {
            MyApplication::Running(*m_CommandList);
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

    //Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "examples/drawmesh/shaders/mesh.vert", {}, ShaderSource_OptimisationPerformance);
    //Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "examples/drawmesh/shaders/mesh.frag", {}, ShaderSource_OptimisationPerformance);
    Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderBinaryFileLoader>(m_CommandList->getContext(), "examples/drawmesh/shaders/mesh.vspv", {});
    Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderBinaryFileLoader>(m_CommandList->getContext(), "examples/drawmesh/shaders/mesh.fspv", {});


    resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("examples/drawmesh/models/voyager/voyager_bc3_unorm.ktx");
    Model* model = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/drawmesh/models/voyager/voyager.dae");

    m_Render = new renderer::SimpleRender(*m_CommandList, m_Window->getSize(), { vertShader, fragShader }, { image }, { model });
    m_Render->setCamera(&m_Camera->getCamera());
}

void MyApplication::Update()
{
    struct
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
        core::Vector4D lightPos = core::Vector4D(25.0f, 5.0f, 5.0f, 1.0f);
    }
    uboVS;

    uboVS.projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
    uboVS.modelMatrix.setTranslation(core::Vector3D(0, 0, 0));
    uboVS.viewMatrix = m_Camera->getCamera().getViewMatrix();

    m_Render->updateParameter(*m_CommandList, "ubo", sizeof(uboVS), &uboVS);
    m_Render->updateParameter(*m_CommandList, "samplerColorMap", 0);
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    m_Camera->update(0);
    Update();

    m_Render->render(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    delete m_Camera;

    delete m_Render;
    delete m_CommandList;
    resource::ResourceLoaderManager::getInstance()->clear();

    stream::StreamManager::clearPools();

    Context::destroyContext(m_Context);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
