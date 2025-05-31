// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"

#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Events/Input/InputEventReceiver.h"

#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Renderer/RenderTargetState.h"

#include "UI/Wigets.h"
#include "UI/WigetGizmo.h"
#include "UI/ImGui/ImGuiHandler.h"

#include "EditorUI.h"
#include "EditorGizmo.h"
#include "EditorScene.h"

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
        , m_EditorScene(new EditorScene())
        , m_EditorGizmo(new EditorGizmo())
    {
        m_Window = Window::createWindow({ 1280, 720 }, { 800, 500 }, false, true, new InputEventReceiver(), "MainWindow");
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

                m_Window->getInputEventReceiver()->resetInputHandlers();
            }
        }

        Exit();
        return 0;
    }

private:

    void Init()
    {
        auto createBackbufferRT = [](Device* device, Swapchain* swapchain) -> renderer::RenderTargetState*
            {
                renderer::RenderTargetState* backbuffer = new renderer::RenderTargetState(device, swapchain->getBackbufferSize());
                backbuffer->setColorTexture(0, swapchain->getBackbuffer(),
                    {
                        renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.f, 0.f, 0.f, 1.f)
                    },
                    {
                        renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
                    }
                );

                return backbuffer;
            };

        InputEventHandler::bind([this, &createBackbufferRT](const SystemEvent* event)
            {
                Window* window = Window::getWindowsByID(event->_windowID);
                if (event->_systemEvent == SystemEvent::Destroy)
                {
                    if (m_Window == window) //Main Window
                    {
                        m_Terminate = true;
                    }
                }
                else if (m_Window == window && event->_systemEvent == SystemEvent::Resize)
                {
                    m_Swapchain->resize(window->getSize());

                    delete m_Backbuffer;
                    m_Backbuffer = createBackbufferRT(m_Device, m_Swapchain);

                }
            });
        std::srand(u32(std::time(0)));

        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::SystemEvent, this);

        //Render
        m_Device = Device::createDevice(Device::RenderType::Vulkan, Device::GraphicMask);
        ASSERT(m_Device, "render is nullptr");

        Swapchain::SwapchainParams params;
        params._size = m_Window->getSize();

        m_Swapchain = m_Device->createSwapchain(m_Window, params);
        m_Backbuffer = createBackbufferRT(m_Device, m_Swapchain);
        ASSERT(m_Backbuffer, "m_Backbuffer is nullptr");

        m_EditorScene->create(m_Device, math::Dimension2D(800, 600));

        //UI
        m_UI = ui::WigetHandler::createWigetHander<ui::ImGuiWigetHandler>(m_Device, m_Backbuffer->getRenderPassDesc(), ui::ImGuiWigetHandler::ImGui_ViewportMode | ui::ImGuiWigetHandler::ImGui_Gizmo);
        //m_UI->showDemoUI();

        InputEventHandler::bind([this](const MouseInputEvent* event)
            {
                m_UI->handleMouseCallback(this, event);
            }
        );

        InputEventHandler::bind([this](const KeyboardInputEvent* event)
            {
                m_UI->handleKeyboardCallback(this, event);
            }
        );

        InputEventHandler::bind([this](const SystemEvent* event)
            {
                m_UI->handleSystemCallback(this, event);
            }
        );

        editor::UI::constructMainMenu(m_UI);

        //Scene Editor
        ui::WigetWindow& sceneEditor = m_UI->createWiget<ui::WigetWindow>("Scene Editor", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .addWiget(ui::WigetText("Mode: "))
            .addWiget(ui::WigetRadioButtonGroup()
                .addElement("select")
                .addElement("move")
                .addElement("rotate")
                .addElement("scale")
                .setActiveIndex(0)
                .setOnChangedIndexEvent([this](ui::Wiget* w, s32 index) -> void
                    {
                        if (index > 0)
                        {
                            m_EditorGizmo->setEnable(true);
                            m_EditorGizmo->setOperation(index - 1);
                        }
                        else
                        {
                            m_EditorGizmo->setEnable(false);
                        }
                    })
                );

        ui::WigetWindow& viewportWin = m_UI->createWiget<ui::WigetWindow>("Viewport", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .setActive(true)
            .setVisible(true)
            .setOnSizeChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Dimension2D& size) -> void
                {
                    LOG_DEBUG("Viewport OnSizeChanged [%d %d]", size._width, size._height);
                })
            .setOnPositionChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Point2D& pos) -> void
                {
                    LOG_DEBUG("Viewport OnPosChanged [%d %d]", pos._x, pos._y);
                })
            .setOnFocusChanged([this](const ui::Wiget* w, bool focused)
                {
                    if (focused)
                    {
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                    }
                    else
                    {
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                    }
                })
            .addWiget(ui::WigetButton("View")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("TODO: Show context menu");
                    })
            )
            .addWiget(ui::WigetLayout(ui::WigetLayout::Border)
                .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentFill)
                .setVAlignment(ui::WigetLayout::VerticalAlignment::AlignmentFill)
                .addWiget(ui::WigetImage(nullptr, {})
                    .setOnUpdate([this](ui::Wiget* w, f32 dt) -> void
                        {
                            auto texture = m_EditorScene->getOutputTexture();
                            static_cast<ui::WigetImage*>(w)->setTexture(texture);
                        })
                    .setOnDrawRectChanged([this](ui::Wiget* w, ui::Wiget* p, const math::Rect& dim) -> void
                        {
                            m_EditorScene->onChanged(dim);

                            auto texture = m_EditorScene->getOutputTexture();
                            static_cast<ui::WigetImage*>(w)->setTexture(texture);
                        })
                )
                .addWiget(ui::WigetGizmo(m_EditorScene->getCamera())
                    .setActive(false)
                    .setOnCreated([this](ui::Wiget* w) -> void
                        {
                            m_EditorGizmo->init(static_cast<ui::WigetGizmo*>(w));
                        })
                    .setOnTransformChangedEvent([this](ui::Wiget* w, ui::Wiget* p, const scene::Transform& tr) -> void
                        {
                            m_EditorGizmo->modify(tr);
                        })
                )
                .addWiget(ui::WigetViewManipulator(m_EditorScene->getCamera())
                    .setOnViewChangedEvent([this](ui::Wiget* w, ui::Wiget* p, const math::Matrix4D& view) -> void
                        {
                            m_EditorScene->onChanged(view);
                        })
                )
            );

        ui::WigetWindow& win1 = m_UI->createWiget<ui::WigetWindow>("Content", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .addWiget(ui::WigetText("text")
            );
        ui::WigetWindow& win2 = m_UI->createWiget<ui::WigetWindow>("Properties", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .addWiget(ui::WigetText("text")
            );
        ui::WigetWindow& win3 = m_UI->createWiget<ui::WigetWindow>("Output", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .addWiget(ui::WigetText("text")
            );

        sceneEditor.setupWindowLayout(ui::WigetWindowLayout(&viewportWin, {
                { ui::WigetWindowLayout::DirLeft, 0.2f, &win1  },
                { ui::WigetWindowLayout::DirRight, 0.2f, &win2 },
                { ui::WigetWindowLayout::DirDown, 0.2f, &win3  } 
            }));

        //editor::UI::constuctTestUIWindow(m_UI, texture);

        //m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);
    }
    
    void Run()
    {
        static u64 s_prevTime = utils::Timer::getCurrentTime();
        const u64 currentTime = utils::Timer::getCurrentTime();
        const f32 diffTime = static_cast<f32>(std::max<s64>(static_cast<s64>(currentTime) - static_cast<s64>(s_prevTime), 0));
        const f32 deltaTime = diffTime / 1'000.f;
        s_prevTime = currentTime;

        //Scene
        {
            m_EditorScene->preRender(deltaTime);
            m_EditorScene->postRender();
            m_EditorScene->submitRender();
        }
        //Scene

        //Editor UI
        {
            renderer::CmdListRender* cmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);

            m_UI->update(m_Window, this, deltaTime);
            m_Swapchain->beginFrame();

            cmdList->beginRenderTarget(*m_Backbuffer);
            if (!m_UI->render(cmdList))
            {
                cmdList->clear(m_Swapchain->getBackbuffer(), { 0, 0, 0, 0 });
            }
            cmdList->endRenderTarget();

            m_Device->submit(cmdList, true);
            m_Device->destroyCommandList(cmdList);

            m_Swapchain->endFrame();
            m_Swapchain->presentFrame();
        }
        //Editor UI
    }

    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent, this);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent, this);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent, this);

        m_EditorScene->destroy();
        delete m_EditorScene;

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
    EditorGizmo*            m_EditorGizmo;

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