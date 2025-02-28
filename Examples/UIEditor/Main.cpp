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
#include "Events/InputEventMouse.h"
#include "Events/InputEventKeyboard.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"
#include "UI/WigetHandler.h"
#include "UI/WigetLayout.h"
#include "UI/Wiget.h"
#include "UI/ImGui.h"

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
        m_Window = Window::createWindow({ 1280, 720 }, { 800, 500 }, false, true, new InputEventReceiver());
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
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, this);
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

        m_EditorScene = new EditorScene();
        m_EditorScene->init(m_Device, m_Swapchain, m_Backbuffer);

        m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);

        m_UI = ui::WigetHandler::createWigetHander<ui::ImGuiWigetHandler>(m_Device, m_CmdList, m_Backbuffer);
        m_UI->showDemoUI();
        InputEventHandler::bind([this](const MouseInputEvent* event)
            {
                m_UI->handleMouseCallback(this, event);
            });
        InputEventHandler::bind([this](const KeyboardInputEvent* event)
            {
                m_UI->handleKeyboardCallback(this, event);
            });

        ui::WigetLayout* UiLayoutMenu = m_UI->createWigetMenuLayout();
        UiLayoutMenu->addWiget<ui::WigetMenu>(new ui::WigetMenu("File"))
            ->addWiget(((new ui::WigetMenu::MenuItem("New"))
                ->setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("New menu click");
                    }))
                )
            ->addWiget(new ui::WigetMenu::MenuItem("Open"))
            ->addWiget(new ui::WigetMenu::MenuItem("Save"))
            ->addWiget(((new ui::WigetMenu("Recent Open"))
                ->addWiget(new ui::WigetMenu::MenuItem("temp.temp")))
                )
            ->addWiget(new ui::WigetMenu::MenuItem("Exit"));

        UiLayoutMenu->addWiget<ui::WigetMenu>(new ui::WigetMenu("Edit"))
            ->setOnClickedEvent([](const ui::Wiget* w) -> void
                {
                    LOG_DEBUG("Edit menu click");
                });
        UiLayoutMenu->addWiget<ui::WigetMenu>(new ui::WigetMenu("View"))
            ->setOnClickedEvent([](const ui::Wiget* w) -> void
                {
                    LOG_DEBUG("View menu click");
                });


        ui::WigetLayout* UiLayout = m_UI->createWigetLayout("Windiw Test", {300, 300}, {500, 200});
        UiLayout->addWiget<ui::WigetButton>(new ui::WigetButton("Button test"))
            ->setOnClickedEvent([](const ui::Wiget* w) -> void
                {
                    const ui::WigetButton* b = static_cast<const ui::WigetButton*>(w);
                    LOG_DEBUG("Button click %s", b->getTitle().c_str());
                })
            ->setActive(true)
            ->setVisible(true);
    }
    
    void Run()
    {
        m_UI->update(m_Window, 0.016f);
        m_EditorScene->update(0.016f);

        //Frame
        m_Swapchain->beginFrame();

        m_CmdList->beginRenderTarget(*m_Backbuffer);

        m_EditorScene->render(m_CmdList);
        m_UI->render(m_CmdList);

        m_CmdList->endRenderTarget();

        m_Device->submit(m_CmdList, false);

        m_Swapchain->endFrame();
        m_Swapchain->presentFrame();
    }

    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent);
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

    ui::ImGuiWigetHandler*  m_UI;
    EditorScene*            m_EditorScene;

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