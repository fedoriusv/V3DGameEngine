// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Input/InputEventReceiver.h"
#include "Resource/ResourceManager.h"
#include "Utils/Logger.h"
#include "Scene.h"

using namespace v3d;
using namespace v3d::renderer;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

class DrawDeshApplication : Application
{
public:

    DrawDeshApplication(int& argc, char** argv) noexcept
        : Application(argc, argv)
        , m_Window(nullptr)
        , m_InputEventHandler(new InputEventHandler())

        , m_Scene(nullptr)
    {
        m_Window = Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver());
        ASSERT(m_Window, "windows is nullptr");
    }

    ~DrawDeshApplication()
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
                app::Scene::HandleCallback(m_Scene, m_InputEventHandler, event);
            });

        m_InputEventHandler->bind([this](const TouchInputEvent* event)
            {
                app::Scene::HandleCallback(m_Scene, m_InputEventHandler, event);
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

        m_Scene = new app::Scene(m_Device, m_Swapchain);
    }

    bool Run()
    {
        f32 dt = 0.16f;

        m_Scene->Run(dt);

        return true;
    }
    
    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent);

        if (m_Scene)
        {
            m_Scene->SendExitSignal();
            m_Scene->Run(0);

            delete m_Scene;
            m_Scene = nullptr;
        }

        if (m_Device)
        {
            m_Device->destroySwapchain(m_Swapchain);

            renderer::Device::destroyDevice(m_Device);
            m_Device = nullptr;
        }
    }

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;
    bool m_terminate = false;

    v3d::renderer::Device* m_Device = nullptr;
    v3d::renderer::Swapchain* m_Swapchain = nullptr;

    app::Scene* m_Scene;
};

int main(int argc, char* argv[])
{
    DrawDeshApplication application(argc, argv);
    return application.execute();
}