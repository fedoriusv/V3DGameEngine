#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::scene;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)
{
    core::Dimension2D widowsSize = { 1280, 720 };
    m_Window = Window::createWindow(widowsSize, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");
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

    m_Scene = new scene::Scene();
    m_Scene->onLoad(*m_CommandList);
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    m_Scene->onUpdate();
    m_Scene->onRender(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    delete m_Scene;

    delete m_CommandList;
    Context::destroyContext(m_Context);
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
