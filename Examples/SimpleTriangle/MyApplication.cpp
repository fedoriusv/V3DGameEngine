#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Formats.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Triangle(new SimpleTriangle())
{
    m_Window = Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([this](const MouseInputEvent* event)
    {
            SimpleTriangle::dispatchEvent(m_Triangle, m_InputEventHandler, event);
    });

    m_InputEventHandler->connect([this](const TouchInputEvent* event)
    {
            SimpleTriangle::dispatchEvent(m_Triangle, m_InputEventHandler, event);
    });

    std::srand(u32(std::time(0)));
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);
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

    m_Triangle->init(m_CommandList, m_Window->getSize());
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    m_Triangle->update(0);
    m_Triangle->render();

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    if (m_Triangle)
    {
        m_Triangle->terminate();

        delete m_Triangle;
        m_Triangle = nullptr;
    }

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);

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
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
