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
#include "Renderer/Texture.h"
#include "UI/WigetHandler.h"
#include "UI/Wigets.h"
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
        m_Window = Window::createWindow({ 1280, 720 }, { 800, 500 }, false, true, new InputEventReceiver(), L"MainWindow");
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
                        renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
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
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, this);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::SystemEvent, this);


        m_Device = Device::createDevice(Device::RenderType::Vulkan, Device::GraphicMask);
        ASSERT(m_Device, "render is nullptr");

        Swapchain::SwapchainParams params;
        params._size = m_Window->getSize();

        m_Swapchain = m_Device->createSwapchain(m_Window, params);
        m_Backbuffer = createBackbufferRT(m_Device, m_Swapchain);
        ASSERT(m_Backbuffer, "m_Backbuffer is nullptr");

        m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);


        m_UI = ui::WigetHandler::createWigetHander<ui::ImGuiWigetHandler>(m_Device, m_CmdList, m_Backbuffer->getRenderPassDesc(), ui::ImGuiWigetHandler::ImGui_EditorMode);
        m_UI->showDemoUI();

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

        m_UI->createWiget<ui::WigetMenuBar>()
            //File
            .addWiget(ui::WigetMenu("File")
                .addWiget(ui::WigetMenuItem("New")
                    .setOnClickedEvent([](const ui::Wiget* w) -> void
                        {
                            LOG_DEBUG("new click");
                        }))
                .addWiget(ui::WigetMenuItem("Open...")
                    .setOnClickedEvent([](const ui::Wiget* w) -> void
                        {
                            LOG_DEBUG("Open click");
                        }))
                .addWiget(ui::WigetMenuItem("Save")
                    .setOnClickedEvent([](const ui::Wiget* w) -> void
                        {
                            LOG_DEBUG("Save click");
                        }))
                .addWiget(ui::WigetMenu("Recent Open")
                    .addWiget(ui::WigetMenuItem("temp.temp"))
                )
                .addWiget(ui::WigetMenuItem("Exit")
                    .setOnClickedEvent([](const ui::Wiget* w) -> void
                        {
                            LOG_DEBUG("Exit click");
                        }))
            )
            //Edit
            .addWiget(ui::WigetMenu("Edit")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Open click");
                    }))
            //View
            .addWiget(ui::WigetMenu("View")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Save click");
                    }))
            .addWiget(ui::WigetMenuItem("TODO")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("TODO click");
                    }))
            .addWiget(ui::WigetMenuItem("FPS:")
                .setOnUpdate([](ui::Wiget* w, f32 dt) -> void
                    {
                        static f32 timePassed = 0;
                        static u32 FPSCounter = 0;

                        const f32 diffTime = dt * 1'000.f;
                        timePassed += diffTime;
                        ++FPSCounter;
                        if (timePassed >= 1'000.f) //sec
                        {
                            static_cast<ui::WigetMenuItem*>(w)->setText(std::format("FPS: {} ({:.2f} ms)", FPSCounter, timePassed / FPSCounter));

                            FPSCounter = 0;
                            timePassed = 0.f;
                        }
                    }));


        //m_UI->createWiget<ui::WigetTabBar>();


        m_UI->createWiget<ui::WigetWindow>("Window Test", math::Dimension2D(300, 600), math::Point2D(10, 10), ui::WigetWindow::Moveable)
            .setActive(true)
            .setVisible(true)
            .setOnActiveChanged([](const ui::Wiget* w) -> void
                {
                    LOG_DEBUG("New menu click");
                })
            .addWiget(ui::WigetButton("button")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Button click");
                    })
                .setToolTip(true, "test tooltip")
            )
            .addWiget(ui::WigetButton("button1")
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Button click");
                    })
            )
            .addWiget(ui::WigetButton("button2")
                .setColor({0.7, 0.5, 0.5, 1.0})
                .setColorHovered({ 0.8, 0.4, 0.4, 1.0 })
                .setColorActive({ 0.9, 0.2, 0.2, 1.0 })
            )
            .addWiget(ui::WigetCheckBox("checkbox", true)
                .setOnChangedValueEvent([](const ui::Wiget* w, bool value) -> void
                    {
                        LOG_DEBUG("Checkbox Value %d", value);
                    })
            )
            .addWiget(ui::WigetRadioButtonGroup()
                .addElement("value 0")
                .addElement("value 1")
                .addElement("value 2")
                .setActiveIndex(1)
                .setOnChangedIndexEvent([](const ui::Wiget* w, s32 index) -> void
                    {
                        LOG_DEBUG("RadioButton Index %d", index);
                    })
            )
            .addWiget(ui::WigetListBox()
                .addElement("AAAA")
                .addElement("BBBB")
                .addElement("CCCC")
                .addElement("DDDD")
                .addElement("FFFF")
                .setActiveIndex(1)
                .setOnChangedIndexEvent([](const ui::Wiget* w, s32 index) -> void
                    {
                        LOG_DEBUG("WigetListBox Index %d", index);
                    })
            )
            .addWiget(ui::WigetComboBox()
                .addElement("AAAA")
                .addElement("BBBB")
                .addElement("CCCC")
                .addElement("DDDD")
                .addElement("FFFF")
                .setActiveIndex(1)
                .setOnChangedIndexEvent([](const ui::Wiget* w, s32 index) -> void
                    {
                        LOG_DEBUG("WigetComboBox Index %d", index);
                    })
            )
            .addWiget(ui::WigetInputField()
                .setActive(true)
            )
            .addWiget(ui::WigetInputSlider()
                .setActive(true)
            );

        math::Dimension2D viewportSize(800, 600);
        ui::WigetWindow& viewport = m_UI->createWiget<ui::WigetWindow>("Viewport", viewportSize, m_Window->getPosition(), ui::WigetWindow::Moveable | ui::WigetWindow::AutoResizeByContent);
        viewport
            .setActive(true)
            .setVisible(true)
            .setOnSizeChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Dimension2D& size) -> void
                {
                    LOG_DEBUG("OnSizeChanged [%d %d]", size.m_width, size.m_height);
                })
            .setOnPositionChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Point2D& pos) -> void
                {
                    LOG_DEBUG("OnPosChanged [%d %d]", pos.m_x, pos.m_y);
                })
            .addWiget(ui::WigetImage(viewportSize)
                .setOnUpdate([this](ui::Wiget* w, f32 dt) -> void
                    {
                        auto texture = m_EditorScene->getOutputTexture();
                        static_cast<ui::WigetImage*>(w)->setTexture(texture);
                        static_cast<ui::WigetImage*>(w)->setSize(texture->getDimension());
                    })
                .setOnDrawRectChanged([this](const ui::Wiget* w, const ui::Wiget* p, const math::Rect32& dim) -> void
                    {
                        const platform::Window* window = static_cast<const ui::WigetWindow*>(p)->getWindow();
                        m_EditorScene->onChanged(window, dim);
                    })
                );
        m_ViewportWiget = &viewport;

        //Editor
        m_EditorScene = new EditorScene();
        m_EditorScene->init(m_Device, viewportSize);

        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
    }
    
    void Run()
    {
        static u64 s_prevTime = 0;
        const u64 currentTime = utils::Timer::getCurrentTime();
        const f32 diffTime = static_cast<f32>(std::max<s64>(static_cast<s64>(currentTime) - static_cast<s64>(s_prevTime), 0));
        const f32 deltaTime = diffTime / 1'000.f;
        s_prevTime = currentTime;


        m_UI->update(m_Window, this, deltaTime);
        m_EditorScene->update(deltaTime);

        //Scene
        m_EditorScene->render(m_CmdList);
        m_Device->submit(m_CmdList, true);

        //UI
        m_Swapchain->beginFrame();

        m_CmdList->beginRenderTarget(*m_Backbuffer);
        m_UI->render(m_CmdList);
        m_CmdList->endRenderTarget();

        m_Device->submit(m_CmdList, true);

        m_Swapchain->endFrame();
        m_Swapchain->presentFrame();
        //UI
    }

    void Exit()
    {
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent, this);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent, this);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent, this);
        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::SystemEvent, this);

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
    const ui::WigetWindow*  m_ViewportWiget;

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