// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"

#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Input/InputEventReceiver.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::renderer;
using namespace v3d::utils;
using namespace v3d::event;

class ClearColorApplication : public v3d::Application, public InputEventHandler
{
public:

    ClearColorApplication(int& argc, char** argv)
        : v3d::Application(argc, argv)
    {
        m_Window = Window::createWindow({ 1024, 768 }, { 800, 500 }, false, false, new InputEventReceiver());
        ASSERT(m_Window, "windows is nullptr");
    }
    
    ~ClearColorApplication()
    {
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

    int execute()
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
        InputEventHandler::bind([this](const MouseInputEvent* event)
            {
                if (event->_event == MouseInputEvent::MousePressDown || event->_event == MouseInputEvent::MouseDoubleClick)
                {
                    s32 rvalue = std::rand();
                    f32 r = 1.0f / RAND_MAX * rvalue;

                    s32 gvalue = std::rand();
                    f32 g = 1.0f / RAND_MAX * gvalue;

                    s32 bvalue = std::rand();
                    f32 b = 1.0f / RAND_MAX * bvalue;

                    m_ClearColor = { r, g, b, 1.0f };

                }
            });

        InputEventHandler::bind([this](const TouchInputEvent* event)
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

                    m_ClearColor = { r, g, b, 1.0f };
                }
            });

        InputEventHandler::bind([this](const SystemEvent* event)
            {
                Window* window = Window::getWindowsByID(event->_windowID);
                if (event->_systemEvent == SystemEvent::Resize)
                {
                    m_Swapchain->resize(window->getSize());
                }
                else if (event->_systemEvent == SystemEvent::Destroy)
                {
                    m_terminate = true;
                }
            });
        std::srand(u32(std::time(0)));

        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::SystemEvent, this);


        m_Device = Device::createDevice(Device::RenderType::Vulkan, Device::GraphicMask);
        ASSERT(m_Device, "render is nullptr");

        Swapchain::SwapchainParams params;
        params._size = m_Window->getSize();

        m_Swapchain = m_Device->createSwapchain(m_Window, params);
        m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);
        m_Sync = m_Device->createSyncPoint(m_CmdList);

        m_ClearColor = { 1.0, 0.0, 0.0, 1.0 };
    }
    
    void Run()
    {
        //Frame
        m_Swapchain->beginFrame();

        m_CmdList->clear(m_Swapchain->getBackbuffer(), m_ClearColor);
        m_Device->submit(m_CmdList, m_Swapchain->getSyncPoint());

        m_Swapchain->endFrame();
        m_Swapchain->presentFrame(m_Sync);
    }

    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent);

        if (m_Device)
        {
            m_Device->destroySyncPoint(m_CmdList, m_Sync);
            m_Device->destroyCommandList(m_CmdList);
            m_Device->destroySwapchain(m_Swapchain);

            renderer::Device::destroyDevice(m_Device);
            m_Device = nullptr;
        }
    }

    v3d::platform::Window* m_Window = nullptr;
    v3d::renderer::Device* m_Device = nullptr;
    bool m_terminate = false;

    v3d::renderer::CmdListRender* m_CmdList = nullptr;
    v3d::renderer::SyncPoint* m_Sync = nullptr;
    v3d::renderer::Swapchain* m_Swapchain = nullptr;

    color::Color m_ClearColor;
};

int main(int argc, char* argv[])
{
    ClearColorApplication app(argc, argv);
    return app.execute();
}