// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"

#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"
#include "Scene/UILayout.h"

#include "Editor.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::renderer;
using namespace v3d::utils;
using namespace v3d::event;


class EditorApplication : public v3d::Application, public InputEventHandler
{
public:

    EditorApplication(int& argc, char** argv)
        : v3d::Application(argc, argv)
    {
        m_Window = Window::createWindow({ 1024, 768 }, { 800, 500 }, false, true, new InputEventReceiver());
        ASSERT(m_Window, "windows is nullptr");
    }
    
    ~EditorApplication()
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
                if (m_Terminate)
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
        InputEventHandler::bind([this](const SystemEvent* event)
            {
                Window* window = Window::getWindowsByID(event->_windowID);
                if (event->_systemEvent == SystemEvent::Destroy)
                {
                    m_Terminate = true;
                }
                else if (event->_systemEvent == SystemEvent::Resize)
                {
                    m_Swapchain->resize(window->getSize());
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

        m_Backbuffer = new renderer::RenderTargetState(m_Device, m_Swapchain->getBackbufferSize());
        m_Backbuffer->setColorTexture(0, m_Swapchain->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
            }
        );

        m_UILayout = new scene::UILayout(m_Device);
        m_UILayout->init(m_Backbuffer);

        m_EditorScene = new EditorScene();
        m_EditorScene->init(m_Device, m_Swapchain, m_Backbuffer);

        m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);
    }
    
    void Run()
    {
        m_UILayout->update(m_Window, this);
        m_EditorScene->update(0.01f);

        //Frame
        m_Swapchain->beginFrame();

        m_CmdList->beginRenderTarget(*m_Backbuffer);

        m_EditorScene->render(m_CmdList);
        m_UILayout->render(m_CmdList);

        m_CmdList->endRenderTarget();

        m_Device->submit(m_CmdList, true);

        m_Swapchain->endFrame();
        m_Swapchain->presentFrame();
    }

    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent);

        if (m_Device)
        {
            m_Device->destroyCommandList(m_CmdList);
            m_Device->destroySwapchain(m_Swapchain);

            renderer::Device::destroyDevice(m_Device);
            m_Device = nullptr;
        }
    }

    v3d::platform::Window* m_Window = nullptr;
    v3d::renderer::Device* m_Device = nullptr;

    scene::UILayout* m_UILayout;
    EditorScene*     m_EditorScene;

    v3d::renderer::CmdListRender* m_CmdList = nullptr;
    v3d::renderer::Swapchain* m_Swapchain = nullptr;

    v3d::renderer::RenderTargetState* m_Backbuffer = nullptr;


    bool m_Terminate = false;
};

int main(int argc, char* argv[])
{
    EditorApplication app(argc, argv);
    return app.execute();
}