#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Object/Texture.h"


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
    m_Window = Window::createWindow({ 1024, 768 }, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([](const KeyboardInputEvent* event)
    {
        if (event->_event == KeyboardInputEvent::KeyboardPressDown)
        {
            LOG_INFO("KeyboardInputEvent Down Key %c modif %x", event->_character, event->_modifers);
        }
        else
        {
            LOG_INFO("KeyboardInputEvent Up Key %c, modif %x", event->_character, event->_modifers);
        }
    });
    m_InputEventHandler->connect([](const MouseInputEvent* event)
    {
        if (event->_event == MouseInputEvent::MousePressDown)
        {
            LOG_INFO("MouseInputEvent Down Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MousePressUp)
        {
            LOG_INFO("MouseInputEvent UP Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseDoubleClick)
        {
            LOG_INFO("MouseInputEvent MouseDoubleClick Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseMoved)
        {
            LOG_INFO("MouseInputEvent MouseMoved Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseWheel)
        {
            LOG_INFO("MouseInputEvent MouseWheel Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
    });

    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
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

        MyApplication::Running(*m_CommandList);
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

    Texture2D texture({0,0}, 0);
    commandList.textureCreate(texture, data);
    commandList.textureUpdate(texture, data);
    commandList.textureSetFilter()

 /*   Texture2D texture = m_CommandList.createTexture({ 0,0 }, 0, nullptr);
    texture.update();*/

    commandList.setPipeline(pipe1);
    commandList.setTexture(texture);
    commandList.setUnifrom(uniform1);
    commandList.draw(geometry1);

    /*Geometry geometry;
    Image image;
    Pipeline pipe;
    {
        texture.upload(image);
        geometry.upload(data);
    }*/
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.cmdBeginFrame();

    /*commandList.set(pipe1);
    commandList.set(texture1);
    commandList.set(uniform1);
    commandList.draw(geometry1);

    commandList.set(pipe2);
    commandList.set(texture2);
    commandList.set(uniform2);
    commandList.draw(geometry2);*/

    commandList.cmdEndFrame();
    commandList.cmdPresentFrame();
    //Frame

    return true;
}

void MyApplication::Exit()
{
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
