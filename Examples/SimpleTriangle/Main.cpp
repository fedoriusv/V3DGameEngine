// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"
#include "Resource/ResourceManager.h"
#include "Utils/Logger.h"

#include "SimpleTriangle.h"

using namespace v3d;
using namespace v3d::renderer;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

class SimpleTriangeApplication : Application
{
public:

    SimpleTriangeApplication(int& argc, char** argv)
        : Application(argc, argv)
        , m_Window(nullptr)
        , m_InputEventHandler(new InputEventHandler())

        , m_Triangle(new SimpleTriangle())
    {
        m_Window = Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver());
        ASSERT(m_Window, "windows is nullptr");
    }
    
    ~SimpleTriangeApplication()
    {
        delete m_InputEventHandler;
        m_InputEventHandler = nullptr;

        if (m_Window)
        {
            event::InputEventReceiver* ieReceiver = m_Window->getInputEventReceiver();

            Window::detroyWindow(m_Window);
            m_Window = nullptr;

            delete ieReceiver;
        }
        Logger::freeInstance();

        memory::memory_test();
    }

    int execute() override
    {
        Init();
        while (true)
        {
            if (!Window::updateEvents(m_Window))
            {
                break;
            }

            if (m_Window->isValid())
            {
                m_Window->getInputEventReceiver()->sendDeferredEvents();
                if (m_terminate)
                {
                    break;
                }
                Run();
            }
        }

        Exit();
        return 0;
    }

private:

    void Init()
    {
        m_InputEventHandler->bind([this](const MouseInputEvent* event)
            {
                SimpleTriangle::dispatchEvent(m_Triangle, m_InputEventHandler, event);
            });

        m_InputEventHandler->bind([this](const TouchInputEvent* event)
            {
                SimpleTriangle::dispatchEvent(m_Triangle, m_InputEventHandler, event);
            });

        m_InputEventHandler->bind([this](const SystemEvent* event)
            {
                Window* window = Window::getWindowsByID(event->_windowID);
                if (event->_systemEvent == SystemEvent::Destroy)
                {
                    m_terminate = true;
                }
            });
        std::srand(u32(std::time(0)));

        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::SystemEvent, m_InputEventHandler);

        m_Device = Device::createDevice(Device::RenderType::Vulkan, Device::GraphicMask);
        ASSERT(m_Device, "render is nullptr");

        Swapchain::SwapchainParams params;
        params._size = m_Window->getSize();

        m_Swapchain = m_Device->createSwapchain(m_Window, params);
        m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);

        m_Triangle->init(m_Device, m_Swapchain);
    }
    
    bool Run()
    {
        f32 dt = 0.f;

        //Frame
        m_Swapchain->beginFrame();

        m_Triangle->update(dt);
        m_Triangle->render();

        m_Swapchain->endFrame();
        m_Swapchain->presentFrame();

        return true;
    }
    
    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent);

        if (m_Triangle)
        {
            m_Triangle->terminate();

            delete m_Triangle;
            m_Triangle = nullptr;
        }

        resource::ResourceManager::getInstance()->clear();
        resource::ResourceManager::getInstance()->freeInstance();

        if (m_Device)
        {
            m_Device->destroyCommandList(m_CmdList);
            m_Device->destroySwapchain(m_Swapchain);

            renderer::Device::destroyDevice(m_Device);
            m_Device = nullptr;
        }
    }

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;
    bool m_terminate = false;

    v3d::renderer::Device* m_Device = nullptr;
    v3d::renderer::CmdListRender* m_CmdList = nullptr;
    v3d::renderer::Swapchain* m_Swapchain = nullptr;

    SimpleTriangle* m_Triangle;
};

int main(int argc, char* argv[])
{
    SimpleTriangeApplication application(argc, argv);
    return application.execute();
}
