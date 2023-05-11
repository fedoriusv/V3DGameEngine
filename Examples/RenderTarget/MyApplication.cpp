#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Event/InputEventReceiver.h"

#include "Renderer/Shader.h"
#include "Resource/Bitmap.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

#include "Scene/Model.h"


using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver()))
    , m_InputEventHandler(new InputEventHandler())

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Scene(nullptr)

{
    ASSERT(m_Window, "windows is nullptr");
}

void MyApplication::Initialize()
{
    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::ImmediateCommandList);

    m_Scene = new SceneRenderer(*m_CommandList);
    m_Scene->Prepare(m_Window->getSize());

    m_InputEventHandler->connect([this](const MouseInputEvent* event)
        {
            m_Scene->getActiveCamera()->handlerMouseCallback(m_InputEventHandler, event);
        });

    m_InputEventHandler->connect([this](const TouchInputEvent* event)
        {
            m_Scene->getActiveCamera()->handlerTouchCallback(m_InputEventHandler, event);
        });

    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);

    m_CommandList->submitCommands(true);
    m_CommandList->flushCommands();
}

bool MyApplication::Running()
{
    f32 dt = 0;
    m_Scene->Render(dt);

    return true;
}

void MyApplication::Exit()
{
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);

    if (m_Scene)
    {
        delete m_Scene;
        m_Scene = nullptr;
    }

    resource::ResourceLoaderManager::getInstance()->clear();
    resource::ResourceLoaderManager::getInstance()->freeInstance();

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

    memory::memory_test();
    return 0;
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);

    utils::Logger::freeInstance();
}
