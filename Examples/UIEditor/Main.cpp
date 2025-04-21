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
#include "Resource/ResourceManager.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Bitmap.h"
#include "UI/Wigets.h"
#include "UI/ImGuiHandler.h"

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

        m_UI = ui::WigetHandler::createWigetHander<ui::ImGuiWigetHandler>(m_Device, m_CmdList, m_Backbuffer->getRenderPassDesc(), ui::ImGuiWigetHandler::ImGui_ViewportMode);
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

        m_UI->createWiget<ui::WigetMenuBar>(ui::WigetMenuBar::MainMenu)
            //File
            .addWiget(ui::WigetMenu("File")
                .addWiget(ui::WigetMenuItem("New")
                    .setOnClickedEvent([](const ui::Wiget* w) -> void
                        {
                            LOG_DEBUG("New click");
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

        //Scene Editor
        ui::WigetWindow& SceneEditor = m_UI->createWiget<ui::WigetWindow>("Scene Editor", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .addWiget(ui::WigetText("Mode: "))
            .addWiget(ui::WigetRadioButtonGroup()
                .addElement("select")
                .addElement("move")
                .addElement("rotate")
                .addElement("scale")
                .setActiveIndex(0)
                .setOnChangedIndexEvent([](const ui::Wiget* w, s32 index) -> void
                    {
                        LOG_DEBUG("Mode Index %d", index);
                    })
                );

        ui::WigetWindow& win0 = m_UI->createWiget<ui::WigetWindow>("Viewport", ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
            .setActive(true)
            .setVisible(true)
            .setOnSizeChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Dimension2D& size) -> void
                {
                    LOG_DEBUG("Viewport OnSizeChanged [%d %d]", size.m_width, size.m_height);
                })
            .setOnPositionChanged([](const ui::Wiget* w, const ui::Wiget* p, const math::Point2D& pos) -> void
                {
                    LOG_DEBUG("Viewport OnPosChanged [%d %d]", pos.m_x, pos.m_y);
                })
            .setOnFocusChanged([this](const ui::Wiget* w, bool focused)
                {
                    if (focused)
                    {
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                    }
                    else
                    {
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent, m_EditorScene);
                        m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent, m_EditorScene);
                    }
                })
            .addWiget(ui::WigetText("text")
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
                    .setOnDrawRectChanged([this](ui::Wiget* w, ui::Wiget* p, const math::Rect32& dim) -> void
                        {
                            m_EditorScene->onChanged(dim);

                            auto texture = m_EditorScene->getOutputTexture();
                            static_cast<ui::WigetImage*>(w)->setTexture(texture);
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

        SceneEditor.setupWindowLayout(ui::WigetWindowLayout(&win0, {
                { ui::WigetWindowLayout::DirLeft, 0.2f, &win1  },
                { ui::WigetWindowLayout::DirRight, 0.2f, &win2 },
                { ui::WigetWindowLayout::DirDown, 0.2f, &win3  } 
            }));

        TestUIWindow();

        //Editor
        m_EditorScene = new EditorScene();
        m_EditorScene->init(m_Device, math::Dimension2D(800, 600));

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
        if (!m_UI->render(m_CmdList))
        {
            m_CmdList->clear(m_Swapchain->getBackbuffer(), { 0, 0, 0, 0 });
        }
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

    void TestUIWindow()
    {

        resource::ResourceManager::getInstance()->addPath("../../../../engine/data/textures/");
        resource::Bitmap* image = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("test_basetex.jpg");
        renderer::Texture2D* texture = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
            image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), image->getMipmapsCount());

        m_CmdList->uploadData(texture, image->getSize(), image->getBitmap());
        m_Device->submit(m_CmdList, true);


        m_UI->createWiget<ui::WigetWindow>("Window Test", math::Dimension2D(800, 800), math::Point2D(10, 10), ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
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
            .addWiget(ui::WigetButton("button1Sized")
                .setSize({ 100, 100 })
                .setOnClickedEvent([](const ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Button click");
                    })
                )
            .addWiget(ui::WigetButton("button2Colored")
                .setColor({ 0.7, 0.5, 0.5, 1.0 })
                .setColorHovered({ 0.8, 0.4, 0.4, 1.0 })
                .setColorActive({ 0.9, 0.2, 0.2, 1.0 })
                )
            .addWiget(ui::WigetText("textColored")
                .setColor({ 0.7, 0.5, 0.5, 1.0 })
                )
            .addWiget(ui::WigetLayout()
                .setFontSize(ui::WigetLayout::LargeFont)
                .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentRight)
                .addWiget(ui::WigetButton("buttonStyled0"))
                .addWiget(ui::WigetText("textStyled0"))
                )
            .addWiget(ui::WigetHorizontalLayout()
                .setFontSize(ui::WigetLayout::MediumFont)
                .addWiget(ui::WigetButton("buttonLine0"))
                .addWiget(ui::WigetButton("buttonLine1"))
                .addWiget(ui::WigetButton("buttonLine2"))
                .addWiget(ui::WigetButton("buttonLine3"))
                )
            .addWiget(ui::WigetLayout(ui::WigetLayout::Border)
                .setSize({ 220, 220 })
                .addWiget(ui::WigetLayout()
                    .setSize({ 200, 100 })
                    .setFontSize(ui::WigetLayout::SmallFont)
                    .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentCenter)
                    .setVAlignment(ui::WigetLayout::VerticalAlignment::AlignmentTop)
                    .addWiget(ui::WigetButton("buttonTop"))
                    .addWiget(ui::WigetText("textTop"))
                    )
                .addWiget(ui::WigetLayout()
                    .setSize({ 200, 100 })
                    .setFontSize(ui::WigetLayout::SmallFont)
                    .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentCenter)
                    .setVAlignment(ui::WigetLayout::VerticalAlignment::AlignmentBottom)
                    .addWiget(ui::WigetButton("buttonBottom"))
                    .addWiget(ui::WigetText("textBottom"))
                    )
                )
            .addWiget(ui::WigetCheckBox("checkbox", true)
                .setOnChangedValueEvent([](const ui::Wiget* w, bool value) -> void
                    {
                        LOG_DEBUG("Checkbox Value %d", value);
                    })
                )
            .addWiget(ui::WigetCheckBox("checkbox", true)
                .setOnChangedValueEvent([](const ui::Wiget* w, bool value) -> void
                    {
                        LOG_DEBUG("Checkbox next Value %d", value);
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
                )
            .addWiget(ui::WigetLayout(ui::WigetLayout::Border)
                .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentFill)
                .addWiget(ui::WigetImage(texture, { 200, 200 }, {0.25, 0.25, 1, 1}))
                );
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