#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTarget.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Formats.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ModelFileLoader.h"

#include "Scene/Model.h"
#include "Scene/Camera.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::resource;
using namespace v3d::scene;

const f32 k_rotationSpeed = 1.5f;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Camera(nullptr)
{
    core::Dimension2D widowsSize = { 1280, 720 };
    m_Window = Window::createWindow(widowsSize, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_Camera = new CameraViewTargetHelper(new Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, 10.0f));
    m_Camera->setPerspective(45.0f, widowsSize, 0.1f, 50.f);

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([this](const MouseInputEvent* event)
    {
        static core::Point2D position = event->_cursorPosition;
        static f32 wheel = event->_wheelValue;

        if (m_InputEventHandler->isLeftMousePressed())
        {
            core::Point2D positionDelta = position - event->_cursorPosition;
            LOG_ERROR("pos %d, %d", positionDelta.x, positionDelta.y);

            core::Vector3D rotation = m_Camera->getRotation();
            rotation.x += positionDelta.y * k_rotationSpeed;
            rotation.y -= positionDelta.x * k_rotationSpeed;
            m_Camera->setRotation(rotation);
            LOG_ERROR("rotate %f, %f", rotation.x, rotation.y);
        }

        if (event->_event == MouseInputEvent::MouseWheel)
        {
            f32 wheelDelta = wheel - event->_wheelValue;
            LOG_ERROR("wheel value %f, delta %f", event->_wheelValue, wheelDelta);

            core::Vector3D postion = m_Camera->getPosition();
            f32 newZPos = postion.z + (wheelDelta * 0.3f);
            postion.z = (newZPos < -5.0f && newZPos > -20.0f) ? newZPos : postion.z;
            m_Camera->setPosition(postion);
        }

        position = event->_cursorPosition;
        wheel = event->_wheelValue;
    });
    m_InputEventHandler->connect(std::bind(&CameraViewTargetHelper::rotateHandler, m_Camera, std::placeholders::_1));

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

    Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "examples/3.simpledraw/shaders/mesh.vert");
    Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "examples/3.simpledraw/shaders/mesh.frag");
    Model* model = resource::ResourceLoaderManager::getInstance()->loadMesh<Model, resource::ModelFileLoader>(m_CommandList->getContext(), "examples/3.simpledraw/models/voyager/voyager.dae");

    m_Render = new renderer::SimpleRender(*m_CommandList, m_Window->getSize(), { vertShader, fragShader }, { model });
    m_Render->setCamera(&m_Camera->getCamera());
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    m_Camera->update();
    m_Render->render(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    delete m_Render;
    delete m_CommandList;
    Context::destroyContext(m_Context);

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
