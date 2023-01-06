#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Texture.h"
#include "Renderer/RenderTargetState.h"
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
{
    m_Window = Window::createWindow({ 1024, 768 }, { 800, 500 }, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([this](const MouseInputEvent* event)
    {
        if (event->_event == MouseInputEvent::MousePressDown || event->_event == MouseInputEvent::MouseDoubleClick)
        {
            s32 rvalue = std::rand();
            f32 r = 1.0f / RAND_MAX * rvalue;

            s32 gvalue = std::rand();
            f32 g = 1.0f / RAND_MAX * gvalue;

            s32 bvalue = std::rand();
            f32 b = 1.0f / RAND_MAX * bvalue;

            m_ClearColor = {r, g, b, 1.0f };

        }
    });

    m_InputEventHandler->connect([this](const TouchInputEvent* event)
    {

        if (event->_event == TouchInputEvent::TouchTypeEvent::TouchMotion && event->_motionEvent == TouchInputEvent::TouchMotionDown)
        {
            LOG_DEBUG("TouchInputEvent event");

            s32 rvalue = std::rand();
            f32 r = 1.0f / RAND_MAX * rvalue;

            s32 gvalue = std::rand();
            f32 g = 1.0f / RAND_MAX * gvalue;

            s32 bvalue = std::rand();
            f32 b = 1.0f / RAND_MAX * bvalue;

            m_ClearColor = {r, g, b, 1.0f };

        }
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
    m_ClearColor = { 1.0, 0.0, 0.0, 1.0 };
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    commandList.clearBackbuffer(m_ClearColor);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
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
    if (m_InputEventHandler)
    {
        delete m_InputEventHandler;
        m_InputEventHandler = nullptr;
    }

    if (m_Window)
    {
        Window::detroyWindow(m_Window);
        m_Window = nullptr;
    }
}
