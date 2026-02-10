// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Application.h"

#include "Utils/Logger.h"
#include "Utils/StringID.h"
#include "Platform/Window.h"
#include "Events/Input/InputEventReceiver.h"
#include "Events/Game/GameEventReceiver.h"

#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Renderer/RenderTargetState.h"

#include "UI/Widgets.h"
#include "UI/WidgetGizmo.h"
#include "UI/ImGui/ImGuiHandler.h"

#include "EditorUI.h"
#include "EditorGizmo.h"
#include "EditorScene.h"
#include "EditorContentScreen.h"
#include "EditorPropertyScreen.h"
#include "EditorAssetBrowser.h"
#include "EditorViewScreen.h"

#include "FrameProfiler.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::renderer;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::color;

class EditorApplication : public v3d::Application, public InputEventHandler
{
public:

    EditorApplication(int& argc, char** argv)
        : v3d::Application(argc, argv)
        , m_UI(nullptr)
        , m_EditorScene(new EditorScene())

        , m_EditorGizmo(new EditorGizmo(m_EditorScene->getGameEventReceiver()))
        , m_EditorContentScreen(new EditorContentScreen(m_EditorScene->getGameEventReceiver()))
        , m_EditorPropertyScreen(new EditorPropertyScreen(m_EditorScene->getGameEventReceiver()))
        , m_EditorAssetBrowser(new EditorAssetBrowser(m_EditorScene->getGameEventReceiver()))
        , m_EditorViewScreen(new EditorViewScreen(m_EditorScene->getGameEventReceiver()))
    {
        m_Window = Window::createWindow({ 1920, 1080 }, { 200, 200 }, false, true, new InputEventReceiver(), "MainWindow");
        ASSERT(m_Window, "windows is nullptr");
    }
    
    ~EditorApplication()
    {
        delete m_EditorGizmo;
        delete m_EditorContentScreen;
        delete m_EditorPropertyScreen;
        delete m_EditorScene;
        delete m_EditorViewScreen;

        if (m_Window)
        {
            InputEventReceiver* inputEventReceiver = m_Window->getInputEventReceiver();
            Window::detroyWindow(m_Window);
            m_Window = nullptr;

            delete inputEventReceiver;
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
                InputEventReceiver* inputEventReceiver = m_Window->getInputEventReceiver();
                inputEventReceiver->sendDeferredEvents();
                if (m_Terminate)
                {
                    break;
                }

                TRACE_PROFILER_FRAME_BEGIN;
                Run();
                TRACE_PROFILER_FRAME_END;

                inputEventReceiver->resetInputHandlers();
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
                        renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_Present
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

        InputEventHandler* inputHandler = m_EditorScene->getInputHandler();
        inputHandler->bind([this](const MouseInputEvent* event)
            {
                m_EditorGizmo->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });
        inputHandler->bind([this](const KeyboardInputEvent* event)
            {
                m_EditorGizmo->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });

        inputHandler->bind([this](const MouseInputEvent* event)
            {
                m_EditorContentScreen->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });
        inputHandler->bind([this](const KeyboardInputEvent* event)
            {
                m_EditorContentScreen->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });

        inputHandler->bind([this](const MouseInputEvent* event)
            {
                m_EditorPropertyScreen->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });
        inputHandler->bind([this](const KeyboardInputEvent* event)
            {
                m_EditorPropertyScreen->handleInputEvent(m_EditorScene->getInputHandler(), event);
            });

        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::SystemEvent, this);


        GameEventHandler* gameHandler = m_EditorScene->getGameHandler();
        gameHandler->bind([this](const GameEvent* event, GameEvent::GameEventType type, u64 ID)
            {
                m_EditorGizmo->handleGameEvent(m_EditorScene->getGameHandler(), event);
            });
        gameHandler->bind([this](const GameEvent* event, GameEvent::GameEventType type, u64 ID)
            {
                m_EditorContentScreen->handleGameEvent(m_EditorScene->getGameHandler(), event);
            });
        gameHandler->bind([this](const GameEvent* event, GameEvent::GameEventType type, u64 ID)
            {
                m_EditorPropertyScreen->handleGameEvent(m_EditorScene->getGameHandler(), event);
            });

        //Render
        m_Device = Device::createDevice(Device::RenderType::Vulkan, Device::GraphicMask);
        ASSERT(m_Device, "render is nullptr");

        Swapchain::SwapchainParams params;
        params._size = m_Window->getSize();
        params._vSync = false;

        m_Swapchain = m_Device->createSwapchain(m_Window, params);
        m_Backbuffer = createBackbufferRT(m_Device, m_Swapchain);
        ASSERT(m_Backbuffer, "m_Backbuffer is nullptr");

        m_EditorScene->createScene(m_Device, m_Window->getSize());
        m_EditorScene->loadScene();

        //UI
        m_UI = ui::WidgetHandler::createWidgetHander<ui::ImGuiWidgetHandler>(m_Device, m_Backbuffer->getRenderPassDesc(), ui::ImGuiWidgetHandler::ImGui_ViewportMode | ui::ImGuiWidgetHandler::ImGui_Gizmo);
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
        ui::WidgetWindow& sceneEditor = m_UI->createWidget<ui::WidgetWindow>("Scene Editor", ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable);

        ui::WidgetPopup& viewPopup = m_UI->createWidget<ui::WidgetPopup>("View");
        m_EditorViewScreen->registerWiget(&viewPopup, m_EditorScene->getSceneData());

        ui::WidgetWindow& viewportWin = m_UI->createWidget<ui::WidgetWindow>("Viewport", ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable)
            .setActive(true)
            .setVisible(true)
            .setOnSizeChanged([](const ui::Widget* w, const ui::Widget* p, const math::Dimension2D& size) -> void
                {
                    LOG_DEBUG("Viewport OnSizeChanged [%d %d]", size._width, size._height);
                })
            .setOnPositionChanged([](const ui::Widget* w, const ui::Widget* p, const math::Point2D& pos) -> void
                {
                    LOG_DEBUG("Viewport OnPosChanged [%d %d]", pos._x, pos._y);
                })
            .setOnFocusChanged([this](const ui::Widget* w, bool focused)
                {
                    InputEventReceiver* inputEventReceiver = m_Window->getInputEventReceiver();
                    if (focused)
                    {
                        inputEventReceiver->attach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene->getInputHandler());
                        inputEventReceiver->attach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene->getInputHandler());
                    }
                    else
                    {
                        inputEventReceiver->dettach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene->getInputHandler());
                        inputEventReceiver->dettach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene->getInputHandler());
                    }
                })
            .addWidget(ui::WidgetHorizontalLayout()
                .addWidget(ui::WidgetButton("View")
                    .setOnClickedEvent([this](const ui::Widget* w) -> void
                        {
                            m_EditorViewScreen->show();
                        })
                )
                .addWidget(ui::WidgetRadioButtonGroup()
                    .addElement("select")
                    .addElement("move")
                    .addElement("rotate")
                    .addElement("scale")
                    .setActiveIndex(0)
                    .setOnChangedIndexEvent([this](ui::Widget* w, s32 index) -> void
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
                )
            )
            .addWidget(ui::WidgetLayout(ui::WidgetLayout::Border)
                .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentFill)
                .setVAlignment(ui::WidgetLayout::VerticalAlignment::AlignmentFill)
                .addWidget(ui::WidgetText("Text"))
                .addWidget(ui::WidgetImage(nullptr, {})
                    .setOnUpdate([this](ui::Widget* w, f32 dt) -> void
                        {
                            auto texture = m_EditorScene->getOutputTexture();
                            static_cast<ui::WidgetImage*>(w)->setTexture(texture);
                        })
                    .setOnDrawRectChanged([this](ui::Widget* w, ui::Widget* p, const math::Rect& dim) -> void
                        {
                            m_EditorScene->onChanged(dim);

                            auto texture = m_EditorScene->getOutputTexture();
                            static_cast<ui::WidgetImage*>(w)->setTexture(texture);
                        })
                )
                .addWidget(ui::WidgetGizmo(m_EditorScene->getCamera())
                    .setActive(false)
                    .setOnCreated([this](ui::Widget* w) -> void
                        {
                            m_EditorGizmo->registerWiget(static_cast<ui::WidgetGizmo*>(w), m_EditorScene->getSceneData());
                        })
                    .setOnTransformChangedEvent([this](ui::Widget* w, ui::Widget* p, const math::Matrix4D& tr) -> void
                        {
                            m_EditorGizmo->modify(tr);
                            m_EditorScene->transformSelectedObject(tr);
                        })
                )
                .addWidget(ui::WidgetViewManipulator(m_EditorScene->getCamera())
                    .setOnViewChangedEvent([this](ui::Widget* w, ui::Widget* p, const math::Matrix4D& view) -> void
                        {
                            m_EditorScene->onChanged(view);
                        })
                )
            );

        ui::WidgetWindow& win1 = m_UI->createWidget<ui::WidgetWindow>("Content", ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable);
        m_EditorContentScreen->registerWiget(&win1, m_EditorScene->getSceneData());

        ui::WidgetWindow& win2 = m_UI->createWidget<ui::WidgetWindow>("Properties", ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable);
        m_EditorPropertyScreen->registerWiget(&win2, m_EditorScene->getSceneData());

        ui::WidgetWindow& win3 = m_UI->createWidget<ui::WidgetWindow>("Assest Browser", ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable);
        m_EditorAssetBrowser->registerWiget(&win3, m_EditorScene->getSceneData());

        sceneEditor.setupWindowLayout(ui::WidgetWindowLayout(&viewportWin, {
                { ui::WidgetWindowLayout::DirLeft, 0.2f, &win1  },
                { ui::WidgetWindowLayout::DirRight, 0.2f, &win2 },
                { ui::WidgetWindowLayout::DirDown, 0.2f, &win3  } 
            }));

        //editor::UI::constuctTestUIWindow(m_UI, m_EditorScene->getSceneData());

        m_cmdListUI = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);
    }
    
    void Run()
    {
        m_EditorScene->beginFrame();

        static u64 s_prevTime = utils::Timer::getCurrentTime();
        const u64 currentTime = utils::Timer::getCurrentTime();
        const f32 diffTime = static_cast<f32>(std::max<s64>(static_cast<s64>(currentTime) - static_cast<s64>(s_prevTime), 0));
        const f32 deltaTime = diffTime / 1'000.f;
        s_prevTime = currentTime;

        //Editor
        {
            TRACE_PROFILER_SCOPE("Update", rgba8::WHITE);
            m_EditorViewScreen->update(deltaTime);
            m_EditorGizmo->update(deltaTime);
            m_EditorContentScreen->update(deltaTime);
            m_EditorPropertyScreen->update(deltaTime);
            m_EditorAssetBrowser->update(deltaTime);
        }

        //Scene
        {
            TRACE_PROFILER_SCOPE("Render", rgba8::WHITE);
            m_EditorScene->preRender(deltaTime);
            m_EditorScene->postRender(deltaTime);
            m_EditorScene->submitRender();
        }
        //Scene

        //Editor UI
        {
            TRACE_PROFILER_SCOPE("Editor UI", rgba8::WHITE);
            m_UI->update(m_Window, this, deltaTime);
            m_Swapchain->beginFrame();

            m_cmdListUI->beginRenderTarget(*m_Backbuffer);
            if (!m_UI->render(m_cmdListUI))
            {
                m_cmdListUI->clear(m_Swapchain->getBackbuffer(), { 0, 0, 0, 0 });
            }
            m_cmdListUI->endRenderTarget();

            m_Device->submit(m_cmdListUI, true);

            m_Swapchain->endFrame();
            m_Swapchain->presentFrame();
        }
        //Editor UI

        m_EditorScene->endFrame();
    }

    void Exit()
    {
        InputEventReceiver* inputEventReceiver = m_Window->getInputEventReceiver();
        inputEventReceiver->dettach(InputEvent::InputEventType::MouseInputEvent, this);
        inputEventReceiver->dettach(InputEvent::InputEventType::KeyboardInputEvent, this);
        inputEventReceiver->dettach(InputEvent::InputEventType::SystemEvent, this);

        m_EditorScene->destroyScene();

        if (m_Device)
        {
            m_Device->destroySwapchain(m_Swapchain);

            m_Device->destroyCommandList(m_cmdListUI);
            renderer::Device::destroyDevice(m_Device);
            m_Device = nullptr;
        }
    }

    v3d::platform::Window* m_Window = nullptr;
    v3d::renderer::Device* m_Device = nullptr;
    v3d::renderer::Swapchain* m_Swapchain = nullptr;
    v3d::renderer::RenderTargetState* m_Backbuffer = nullptr;
    v3d::renderer::CmdListRender* m_cmdListUI = nullptr;

    ui::ImGuiWidgetHandler* m_UI;
    EditorScene*            m_EditorScene;

    EditorGizmo*            m_EditorGizmo;
    EditorContentScreen*    m_EditorContentScreen;
    EditorPropertyScreen*   m_EditorPropertyScreen;
    EditorAssetBrowser*     m_EditorAssetBrowser;
    EditorViewScreen*       m_EditorViewScreen;

    bool m_Terminate = false;
};

int main(int argc, char* argv[])
{
    EditorApplication app(argc, argv);
    return app.execute();
}