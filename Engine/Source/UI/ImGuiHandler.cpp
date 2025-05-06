#include "ImGuiHandler.h"
#include "Wiget.h"

#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Platform/Platform.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Input/InputEventReceiver.h"
#include "Events/Input/InputEventMouse.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Stream/StreamManager.h"
#include "Resource/ShaderCompiler.h"
#include "UI/Wigets.h"

#if USE_IMGUI
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"
#include "ImGuiFonts.h"
#include "ImGuiViewport.h"
#include "ImGuiDrawer.h"

namespace v3d
{
namespace ui
{

static ImGuiKey KeyEventToImGuiKey(event::KeyCode key)
{
    switch (key)
    {
    case event::KeyCode::KeyTab: return ImGuiKey_Tab;
    case event::KeyCode::KeyLeft: return ImGuiKey_LeftArrow;
    case event::KeyCode::KeyRight: return ImGuiKey_RightArrow;
    case event::KeyCode::KeyUp: return ImGuiKey_UpArrow;
    case event::KeyCode::KeyDown: return ImGuiKey_DownArrow;
    case event::KeyCode::KeyPrior: return ImGuiKey_PageUp;
    case event::KeyCode::KeyNext: return ImGuiKey_PageDown;
    case event::KeyCode::KeyHome: return ImGuiKey_Home;
    case event::KeyCode::KeyEnd: return ImGuiKey_End;
    case event::KeyCode::KeyInsert: return ImGuiKey_Insert;
    case event::KeyCode::KeyDelete: return ImGuiKey_Delete;
    case event::KeyCode::KeyBackspace: return ImGuiKey_Backspace;
    case event::KeyCode::KeySpace: return ImGuiKey_Space;
    case event::KeyCode::KeyReturn: return ImGuiKey_Enter;
    case event::KeyCode::KeyEscape: return ImGuiKey_Escape;
    case event::KeyCode::KeyComma: return ImGuiKey_Comma;
    case event::KeyCode::KeyPeriod: return ImGuiKey_Period;
    case event::KeyCode::KeyCapital: return ImGuiKey_CapsLock;
    case event::KeyCode::KeyScroll: return ImGuiKey_ScrollLock;
    case event::KeyCode::KeyNumlock: return ImGuiKey_NumLock;
    case event::KeyCode::KeySnapshot: return ImGuiKey_PrintScreen;
    case event::KeyCode::KeyPause: return ImGuiKey_Pause;
    case event::KeyCode::KeyNumpad0: return ImGuiKey_Keypad0;
    case event::KeyCode::KeyNumpad1: return ImGuiKey_Keypad1;
    case event::KeyCode::KeyNumpad2: return ImGuiKey_Keypad2;
    case event::KeyCode::KeyNumpad3: return ImGuiKey_Keypad3;
    case event::KeyCode::KeyNumpad4: return ImGuiKey_Keypad4;
    case event::KeyCode::KeyNumpad5: return ImGuiKey_Keypad5;
    case event::KeyCode::KeyNumpad6: return ImGuiKey_Keypad6;
    case event::KeyCode::KeyNumpad7: return ImGuiKey_Keypad7;
    case event::KeyCode::KeyNumpad8: return ImGuiKey_Keypad8;
    case event::KeyCode::KeyNumpad9: return ImGuiKey_Keypad9;
    case event::KeyCode::KeyDecimal: return ImGuiKey_KeypadDecimal;
    case event::KeyCode::KeyDivide: return ImGuiKey_KeypadDivide;
    case event::KeyCode::KeyMultiply: return ImGuiKey_KeypadMultiply;
    case event::KeyCode::KeySubtract: return ImGuiKey_KeypadSubtract;
    case event::KeyCode::KeyAdd: return ImGuiKey_KeypadAdd;
    case event::KeyCode::KeyLShift: return ImGuiKey_LeftShift;
    case event::KeyCode::KeyLControl: return ImGuiKey_LeftCtrl;
    case event::KeyCode::KeyLAlt: return ImGuiKey_LeftAlt;
    case event::KeyCode::KeyLWin: return ImGuiKey_LeftSuper;
    case event::KeyCode::KeyRShift: return ImGuiKey_RightShift;
    case event::KeyCode::KeyRControl: return ImGuiKey_RightCtrl;
    case event::KeyCode::KeyRAlt: return ImGuiKey_RightAlt;
    case event::KeyCode::KeyRWin: return ImGuiKey_RightSuper;
    case event::KeyCode::KeyApps: return ImGuiKey_Menu;
    case event::KeyCode::KeyKey_0: return ImGuiKey_0;
    case event::KeyCode::KeyKey_1: return ImGuiKey_1;
    case event::KeyCode::KeyKey_2: return ImGuiKey_2;
    case event::KeyCode::KeyKey_3: return ImGuiKey_3;
    case event::KeyCode::KeyKey_4: return ImGuiKey_4;
    case event::KeyCode::KeyKey_5: return ImGuiKey_5;
    case event::KeyCode::KeyKey_6: return ImGuiKey_6;
    case event::KeyCode::KeyKey_7: return ImGuiKey_7;
    case event::KeyCode::KeyKey_8: return ImGuiKey_8;
    case event::KeyCode::KeyKey_9: return ImGuiKey_9;
    case event::KeyCode::KeyKey_A: return ImGuiKey_A;
    case event::KeyCode::KeyKey_B: return ImGuiKey_B;
    case event::KeyCode::KeyKey_C: return ImGuiKey_C;
    case event::KeyCode::KeyKey_D: return ImGuiKey_D;
    case event::KeyCode::KeyKey_E: return ImGuiKey_E;
    case event::KeyCode::KeyKey_F: return ImGuiKey_F;
    case event::KeyCode::KeyKey_G: return ImGuiKey_G;
    case event::KeyCode::KeyKey_H: return ImGuiKey_H;
    case event::KeyCode::KeyKey_I: return ImGuiKey_I;
    case event::KeyCode::KeyKey_J: return ImGuiKey_J;
    case event::KeyCode::KeyKey_K: return ImGuiKey_K;
    case event::KeyCode::KeyKey_L: return ImGuiKey_L;
    case event::KeyCode::KeyKey_M: return ImGuiKey_M;
    case event::KeyCode::KeyKey_N: return ImGuiKey_N;
    case event::KeyCode::KeyKey_O: return ImGuiKey_O;
    case event::KeyCode::KeyKey_P: return ImGuiKey_P;
    case event::KeyCode::KeyKey_Q: return ImGuiKey_Q;
    case event::KeyCode::KeyKey_R: return ImGuiKey_R;
    case event::KeyCode::KeyKey_S: return ImGuiKey_S;
    case event::KeyCode::KeyKey_T: return ImGuiKey_T;
    case event::KeyCode::KeyKey_U: return ImGuiKey_U;
    case event::KeyCode::KeyKey_V: return ImGuiKey_V;
    case event::KeyCode::KeyKey_W: return ImGuiKey_W;
    case event::KeyCode::KeyKey_X: return ImGuiKey_X;
    case event::KeyCode::KeyKey_Y: return ImGuiKey_Y;
    case event::KeyCode::KeyKey_Z: return ImGuiKey_Z;
    case event::KeyCode::KeyF1: return ImGuiKey_F1;
    case event::KeyCode::KeyF2: return ImGuiKey_F2;
    case event::KeyCode::KeyF3: return ImGuiKey_F3;
    case event::KeyCode::KeyF4: return ImGuiKey_F4;
    case event::KeyCode::KeyF5: return ImGuiKey_F5;
    case event::KeyCode::KeyF6: return ImGuiKey_F6;
    case event::KeyCode::KeyF7: return ImGuiKey_F7;
    case event::KeyCode::KeyF8: return ImGuiKey_F8;
    case event::KeyCode::KeyF9: return ImGuiKey_F9;
    case event::KeyCode::KeyF10: return ImGuiKey_F10;
    case event::KeyCode::KeyF11: return ImGuiKey_F11;
    case event::KeyCode::KeyF12: return ImGuiKey_F12;
    case event::KeyCode::KeyF13: return ImGuiKey_F13;
    case event::KeyCode::KeyF14: return ImGuiKey_F14;
    case event::KeyCode::KeyF15: return ImGuiKey_F15;
    case event::KeyCode::KeyF16: return ImGuiKey_F16;
    case event::KeyCode::KeyF17: return ImGuiKey_F17;
    case event::KeyCode::KeyF18: return ImGuiKey_F18;
    case event::KeyCode::KeyF19: return ImGuiKey_F19;
    case event::KeyCode::KeyF20: return ImGuiKey_F20;
    case event::KeyCode::KeyF21: return ImGuiKey_F21;
    case event::KeyCode::KeyF22: return ImGuiKey_F22;
    case event::KeyCode::KeyF23: return ImGuiKey_F23;
    case event::KeyCode::KeyF24: return ImGuiKey_F24;
    default: break;
    }

    return ImGuiKey_None;
}

ImGuiWigetHandler::ImGuiWigetHandler(renderer::Device* device, ImGuiWigetFlags flags) noexcept
    : WigetHandler(device)
    , m_ImGuiContext(nullptr)
    , m_showDemo(false)

    , m_UIProgram(nullptr)
    , m_UIPipeline(nullptr)

    , m_viewportData(V3D_NEW(ImGuiWigetViewportData, memory::MemoryLabel::MemoryUI)())
    , m_frameCounter(~1)
    , m_flags(flags)
{
    m_uiDrawer = V3D_NEW(ImGuiWigetDrawer, memory::MemoryLabel::MemoryUI)(this);
}

ImGuiWigetHandler::~ImGuiWigetHandler()
{
    V3D_DELETE(m_uiDrawer, memory::MemoryLabel::MemoryUI);
    m_uiDrawer = nullptr;

    ASSERT(m_viewportData->_indexBuffer.empty(), "must be empty");
    ASSERT(m_viewportData->_vertexBuffer.empty(), "must be empty");
    ASSERT(m_viewportData->_geometryDesc.empty(), "must be empty");
    V3D_DELETE(m_viewportData, memory::MemoryLabel::MemoryUI);

    ASSERT(!m_ImGuiContext, "must be nullptr");
}

bool ImGuiWigetHandler::create(renderer::CmdListRender* cmdList, const renderer::RenderPassDesc& renderpassDesc)
{
    ASSERT(!m_ImGuiContext, "must be nullptr");
    ImGui::SetAllocatorFunctions(
        [](size_t size, void* userData) -> void*
        {
            return V3D_MALLOC(size, v3d::memory::MemoryLabel::MemorySystem);
        },
        [](void* ptr, void* userData)
        {
            V3D_FREE(ptr, v3d::memory::MemoryLabel::MemorySystem);
        }
    );

    m_ImGuiContext = ImGui::CreateContext();
    if (!m_ImGuiContext)
    {
        LOG_ERROR("ImGuiWigetLayout::create CreateContext is failed");
        destroy();

        return false;
    }

    ImGuiIO& imguiIO = ImGui::GetIO();

    imguiIO.BackendPlatformUserData = this;
    imguiIO.BackendPlatformName = "ImGui";
    imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    imguiIO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    imguiIO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    imguiIO.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

    imguiIO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    if (m_flags && ImGuiWigetFlag::ImGui_ViewportMode)
    {
        imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        imguiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        imguiIO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
        imguiIO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
        imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
        imguiIO.ConfigViewportsNoDefaultParent = false;
    }

    ImGuiPlatformIO& imguiPlatformIO = ImGui::GetPlatformIO();
    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Window
        imguiPlatformIO.Platform_CreateWindow = ImGuiWigetViewportEvents::ImGui_CreateWindow;
        imguiPlatformIO.Platform_DestroyWindow = ImGuiWigetViewportEvents::ImGui_DestroyWindow;
        imguiPlatformIO.Platform_ShowWindow = ImGuiWigetViewportEvents::ImGui_ShowWindow;
        imguiPlatformIO.Platform_SetWindowPos = ImGuiWigetViewportEvents::ImGui_SetWindowPos;
        imguiPlatformIO.Platform_GetWindowPos = ImGuiWigetViewportEvents::ImGui_GetWindowPos;
        imguiPlatformIO.Platform_SetWindowSize = ImGuiWigetViewportEvents::ImGui_SetWindowSize;
        imguiPlatformIO.Platform_GetWindowSize = ImGuiWigetViewportEvents::ImGui_GetWindowSize;
        imguiPlatformIO.Platform_SetWindowFocus = ImGuiWigetViewportEvents::ImGui_SetWindowFocus;
        imguiPlatformIO.Platform_GetWindowFocus = ImGuiWigetViewportEvents::ImGui_GetWindowFocus;
        imguiPlatformIO.Platform_GetWindowMinimized = ImGuiWigetViewportEvents::ImGui_GetWindowMinimized;
        imguiPlatformIO.Platform_SetWindowTitle = ImGuiWigetViewportEvents::ImGui_SetWindowTitle;
        imguiPlatformIO.Platform_SetWindowAlpha = ImGuiWigetViewportEvents::ImGui_SetWindowAlpha;
        imguiPlatformIO.Platform_UpdateWindow = ImGuiWigetViewportEvents::ImGui_UpdateWindow;
        imguiPlatformIO.Platform_GetWindowDpiScale = ImGuiWigetViewportEvents::ImGui_GetWindowDpiScale;
        imguiPlatformIO.Platform_OnChangedViewport = ImGuiWigetViewportEvents::ImGui_OnChangedViewport;

        // Render
        imguiPlatformIO.Renderer_CreateWindow = ImGuiWigetViewportEvents::ImGui_Renderer_CreateWindow;
        imguiPlatformIO.Renderer_DestroyWindow = ImGuiWigetViewportEvents::ImGui_Renderer_DestroyWindow;
        imguiPlatformIO.Renderer_SetWindowSize = ImGuiWigetViewportEvents::ImGui_Renderer_SetWindowSize;
        imguiPlatformIO.Renderer_RenderWindow = ImGuiWigetViewportEvents::ImGui_Renderer_RenderWindow;
        imguiPlatformIO.Renderer_SwapBuffers = ImGuiWigetViewportEvents::ImGui_Renderer_Present;
    }

    auto displayMonitors = [](const math::RectF32& rcMonitor, const math::RectF32& rcWork, f32 dpi, bool primary, void* monitor) -> bool
        {
            ImGuiPlatformMonitor imguiMonitor;
            imguiMonitor.MainPos = ImVec2(rcMonitor.getLeftX(), rcMonitor.getTopY());
            imguiMonitor.MainSize = ImVec2(rcMonitor.getRightX() - rcMonitor.getLeftX(), rcMonitor.getBottomY() - rcMonitor.getTopY());
            imguiMonitor.WorkPos = ImVec2(rcWork.getLeftX(), rcWork.getTopY());
            imguiMonitor.WorkSize = ImVec2(rcWork.getRightX() - rcWork.getLeftX(), rcWork.getBottomY() - rcWork.getTopY());
            imguiMonitor.DpiScale = dpi;
            imguiMonitor.PlatformHandle = monitor;
            if (imguiMonitor.DpiScale <= 0.0f)
            {
                return true;
            }

            ImGuiPlatformIO& imguiPlatformIO = ImGui::GetPlatformIO();
            if (primary)
            {
                imguiPlatformIO.Monitors.push_front(imguiMonitor);
            }
            else
            {
                imguiPlatformIO.Monitors.push_back(imguiMonitor);
            }

            return true;
        };

    imguiPlatformIO.Monitors.resize(0);
    platform::Platform::enumDisplayMonitors(displayMonitors);


    //Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.9f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.9f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.62f, 0.85f, 1.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.50f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
    style.Alpha = 1.0f;
    style.FrameRounding = 4;
    style.IndentSpacing = 12.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = { 8.0f, 8.0f };
    style.WindowTitleAlign = { 0.5f, 0.5f };
    style.WindowMenuButtonPosition = ImGuiDir_None;
    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float);
    ImGui::StyleColorsDark();

    ImFontConfig fontConfig{};
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 3;
    fontConfig.PixelSnapH = true;
    if (!createFontTexture(cmdList, &fontConfig))
    {
        LOG_ERROR("ImGuiWigetLayout::create createFontTexture is failed");
        destroy();

        return false;
    }

    if (!createPipeline(renderpassDesc))
    {
        LOG_ERROR("ImGuiWigetLayout::create createPipeline is failed");
        destroy();

        return false;
    }

    if (!createBuffers(m_viewportData, k_ImGui_IndexCount, k_ImGui_VertexCount))
    {
        LOG_ERROR("ImGuiWigetLayout::create createBuffers is failed");
        destroy();

        return false;
    }

    //ImGui::StyleColorsClassic();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Color scheme
    //style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    //style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    //style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    //style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    //style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    //ImGui::SetCurrentContext(m_IMGUIContext);

    return true;
}

void ImGuiWigetHandler::destroy()
{
    ImGuiIO& imguiIO = ImGui::GetIO();
    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::DestroyPlatformWindows();
    }

    destroyBuffers(m_viewportData);
    destroyPipeline();
    destroyFontTexture();

    if (m_ImGuiContext)
    {
        ImGui::DestroyContext(m_ImGuiContext);
        m_ImGuiContext = nullptr;
    }
}

void ImGuiWigetHandler::handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    //ImGuiIO& imguiIO = ImGui::GetIO();
    //imguiIO.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
    //imguiIO.AddMouseButtonEvent(0, handler->isLeftMousePressed());
    //imguiIO.AddMouseButtonEvent(1, handler->isRightMousePressed());
    //imguiIO.AddMouseWheelEvent(0.0f, handler->getMouseWheel());
}

void ImGuiWigetHandler::handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    ImGuiKey key = KeyEventToImGuiKey(event->_key);
    const s32 native_scancode = (int)event->_character + 30;

    if (key != ImGuiKey_None)
    {
        ImGuiIO& imguiIO = ImGui::GetIO();
        imguiIO.AddKeyEvent(key, event->_event == event::KeyboardInputEvent::KeyboardPressDown);
        imguiIO.SetKeyEventNativeData(key, event->_character, native_scancode); // To support legacy indexing (<1.87 user code)
    }
}

void ImGuiWigetHandler::handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");
}

void ImGuiWigetHandler::handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    ImGuiIO& imguiIO = ImGui::GetIO();

    if (event->_systemEvent == event::SystemEvent::Focus)
    {
        if (!(imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
        {
            imguiIO.AddFocusEvent(event->_flag);
        }
    }
    else if (event->_systemEvent == event::SystemEvent::TextInput)
    {
        imguiIO.AddInputCharacter(event->_flag);
    }
}

void ImGuiWigetHandler::update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.DeltaTime = (dt == 0.0f) ? 1.0f / 60.0f : dt;
    imguiIO.DisplaySize.x = window->getSize().m_width;
    imguiIO.DisplaySize.y = window->getSize().m_height;
    imguiIO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    imguiIO.FontDefault = m_fonts[WigetLayout::MediumFont];

    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        viewport->PlatformHandle = window->getWindowHandle();
        viewport->PlatformHandleRaw = window->getWindowHandle();
        viewport->PlatformUserData = const_cast<platform::Window*>(window);
    }

    if (handler->isMouseInputEventsBound())
    {
        math::Point2D cursorPosition{};
        const platform::Window* activeWindow = window;
        if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
            cursorPosition = platform::Platform::getCursorPosition();

            ImGuiID mouse_viewport_id = 0;
            //TODO WindowFromPoint
            if (HWND hovered_hwnd = ::WindowFromPoint({ cursorPosition.m_x, cursorPosition.m_y}))
            {
                for (ImGuiViewport* viewport : platform_io.Viewports)
                {
                    if (viewport->PlatformHandle == hovered_hwnd)
                    {
                        mouse_viewport_id = viewport->ID;
                    }
                }
            }

            imguiIO.AddMouseViewportEvent(mouse_viewport_id);
        }
        else
        {
            cursorPosition = handler->getRelativeCursorPosition();
        }

        imguiIO.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        imguiIO.AddMousePosEvent(cursorPosition.m_x, cursorPosition.m_y);
        imguiIO.AddMouseButtonEvent(0, handler->isLeftMousePressed());
        imguiIO.AddMouseButtonEvent(1, handler->isRightMousePressed());
        imguiIO.AddMouseWheelEvent(0.0f, handler->getMouseWheel());
    }

    if (handler->isKeyboardInputEventsBound())
    {
        imguiIO.AddKeyEvent(ImGuiMod_Ctrl, handler->isKeyPressed(event::KeyCode::KeyControl));
        imguiIO.AddKeyEvent(ImGuiMod_Shift, handler->isKeyPressed(event::KeyCode::KeyShift));
        imguiIO.AddKeyEvent(ImGuiMod_Alt, handler->isKeyPressed(event::KeyCode::KeyAlt));
        imguiIO.AddKeyEvent(ImGuiMod_Super, handler->isKeyPressed(event::KeyCode::KeyLWin) || handler->isKeyPressed(event::KeyCode::KeyRWin));
    }

    //update text list
    m_activeTextures.clear();
    m_activeTextures.push_back(m_fontAtlas);

    ImGui::NewFrame();
    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    }

    if (m_flags && ImGuiWigetFlag::ImGui_Gizmo)
    {
        ImGuizmo::BeginFrame();
    }

    WigetHandler::update(window, handler, dt);

    if (m_showDemo)
    {
        ImGui::ShowDemoWindow();
    }

    ImGui::Render();

    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }
}

void ImGuiWigetHandler::showDemoUI()
{
    m_showDemo = true;
}

void ImGuiWigetHandler::hideDemoUI()
{
    m_showDemo = false;
}

bool ImGuiWigetHandler::createFontTexture(renderer::CmdListRender* cmdList, ImFontConfig* fontConfig)
{
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.Fonts->ClearFonts();

    const bool forceDefault = false;
    if (forceDefault)
    {
        ImFont* defafultFont = imguiIO.Fonts->AddFontDefault(fontConfig);
        m_fonts.fill(defafultFont);
    }
    else
    {
        m_fonts[WigetLayout::SmallFont] = imguiIO.Fonts->AddFontFromMemoryCompressedTTF(k_source_sans_pro_regular_compressed_data, k_source_sans_pro_regular_compressed_size, 13, fontConfig, imguiIO.Fonts->GetGlyphRangesDefault());
        m_fonts[WigetLayout::MediumFont] = imguiIO.Fonts->AddFontFromMemoryCompressedTTF(k_source_sans_pro_regular_compressed_data, k_source_sans_pro_regular_compressed_size, 20, fontConfig, imguiIO.Fonts->GetGlyphRangesDefault());
        m_fonts[WigetLayout::LargeFont] = imguiIO.Fonts->AddFontFromMemoryCompressedTTF(k_source_sans_pro_regular_compressed_data, k_source_sans_pro_regular_compressed_size, 32, fontConfig, imguiIO.Fonts->GetGlyphRangesDefault());
    }
    imguiIO.FontGlobalScale = 1.0f;

    //Font texture
    s32 width, height;
    u8* pixels = nullptr;
    u32 pixelSize = 4; //RGBA8
    imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_imageSampler = V3D_NEW(renderer::SamplerState, memory::MemoryLabel::MemoryRenderCore)(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    if (!m_imageSampler)
    {
        return false;
    }
    m_imageSampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);
    m_imageSampler->setBorderColor(math::Vector4D(1.0, 1.0, 1.0, 1.0)); //opaque white

    m_fontAtlas = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryRenderCore)(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Write, renderer::Format::Format_R8G8B8A8_UNorm, math::Dimension2D(width, height), 1);
    if (!m_fontAtlas)
    {
        return false;
    }
    imguiIO.Fonts->SetTexID(0); //reserve for m_fontAtlas

    cmdList->uploadData(m_fontAtlas, width * height * pixelSize, pixels);
    m_device->submit(cmdList, true);

    return true;
}

void ImGuiWigetHandler::destroyFontTexture()
{
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.Fonts->ClearFonts();

    if (m_imageSampler)
    {
        V3D_DELETE(m_imageSampler, memory::MemoryLabel::MemoryRenderCore);
        m_imageSampler = nullptr;
    }

    if (m_fontAtlas)
    {
        V3D_DELETE(m_fontAtlas, memory::MemoryLabel::MemoryRenderCore);
        m_fontAtlas = nullptr;
    }
}

bool ImGuiWigetHandler::createBuffers(ImGuiWigetViewportData* viewportData, u32 indexCount, u32 vertexCount)
{
    viewportData->_indexBuffer.resize(k_countSwapchaints);
    viewportData->_vertexBuffer.resize(k_countSwapchaints);
    viewportData->_geometryDesc.resize(k_countSwapchaints);

    for (u32 i = 0; i < k_countSwapchaints; ++i)
    {
        viewportData->_indexBuffer[i] = new renderer::IndexBuffer(m_device, renderer::BufferUsage::Buffer_GPUWriteCocherent, sizeof(ImDrawIdx) == 2 ? renderer::IndexBufferType::IndexType_16 : renderer::IndexBufferType::IndexType_32, indexCount, "UIIndexBuffer_" + std::to_string(i));
        viewportData->_vertexBuffer[i] = new renderer::VertexBuffer(m_device, renderer::BufferUsage::Buffer_GPUWriteCocherent, vertexCount, vertexCount * sizeof(ImDrawVert), "UIVertexBuffer_" + std::to_string(i));
        viewportData->_geometryDesc[i] = renderer::GeometryBufferDesc(viewportData->_indexBuffer[i], 0, viewportData->_vertexBuffer[i], 0, sizeof(ImDrawVert), 0);
    }

    return true;
}

void ImGuiWigetHandler::destroyBuffers(ImGuiWigetViewportData* viewportData)
{
    for (u32 i = 0; i < k_countSwapchaints; ++i)
    {
        delete viewportData->_indexBuffer[i];
        delete viewportData->_vertexBuffer[i];
    }

    viewportData->_indexBuffer.clear();
    viewportData->_vertexBuffer.clear();
    viewportData->_geometryDesc.clear();
}

bool ImGuiWigetHandler::createPipeline(const renderer::RenderPassDesc& renderpassDesc)
{
    //shaders
    {
        const renderer::VertexShader* vertShader = nullptr;
        {
            const std::string vertexSource("\
            struct VS_INPUT\n\
            {\n\
                [[vk::location(0)]] float2 Position : POSITION;\n\
                [[vk::location(1)]] float2 UV       : TEXTURE;\n\
                [[vk::location(2)]] float4 Color    : COLOR;\n\
            };\n\
            \n\
            struct PushConstants\n\
            {\n\
                float2 scale;\n\
                float2 translate;\n\
            };\n\
            \n\
            [[vk::push_constant]]\n\
            PushConstants pushConstants;\n\
            \n\
            struct VS_OUTPUT\n\
            {\n\
                float4 Pos                       : SV_POSITION;\n\
                [[vk::location(0)]] float2 UV    : TEXTURE;\n\
                [[vk::location(1)]] float4 Color : COLOR;\n\
            };\n\
            \n\
            VS_OUTPUT main(VS_INPUT Input)\n\
            {\n\
                VS_OUTPUT Out;\n\
                Out.Pos = float4(Input.Position * pushConstants.scale + pushConstants.translate, 0.0, 1.0);\n\
                Out.Pos.y = -Out.Pos.y;\n\
                Out.UV = Input.UV;\n\
                Out.Color = Input.Color;\n\
                return Out;\n\
            }");
            const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

            resource::ShaderDecoder::ShaderPolicy vertexPolicy;
            vertexPolicy._type = renderer::ShaderType::Vertex;
            vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            vertexPolicy._content = renderer::ShaderContent::Source;
            vertexPolicy._entryPoint = "main";

            vertShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(m_device, "UIVertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(vertexStream);
        }

        const renderer::FragmentShader* fragShader = nullptr;
        {
            const std::string fragmentSource("\
            struct PS_INPUT\n\
            {\n\
                [[vk::location(0)]] float2 UV    : TEXTURE;\n\
                [[vk::location(1)]] float4 Color : COLOR;\n\
            };\n\
            \n\
            [[vk::binding(0, 0)]] Texture2D fontTexture    : register(t0);\n\
            [[vk::binding(1, 0)]] SamplerState fontSampler : register(s0);\n\
            \n\
            float4 main(PS_INPUT Input) : SV_TARGET0\n\
            {\n\
                return Input.Color * fontTexture.Sample(fontSampler, Input.UV);\n\
            }");
            const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

            resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
            fragmentPolicy._type = renderer::ShaderType::Fragment;
            fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            fragmentPolicy._content = renderer::ShaderContent::Source;
            fragmentPolicy._entryPoint = "main";

            fragShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(m_device, "UIFragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(fragmentStream);
        }

        ASSERT(vertShader && fragShader, "nullptr");
        m_UIProgram = V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryRenderCore)(m_device, vertShader, fragShader);
        if (!m_UIProgram)
        {
            return false;
        }
    }

    //pipeline
    {
        renderer::VertexInputAttributeDesc vertexDesc(
            {
                renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(ImDrawVert)),
            },
            {
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(ImDrawVert, pos)),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(ImDrawVert, uv)),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R8G8B8A8_UNorm, offsetof(ImDrawVert, col)),
            }
        );

        m_UIPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryRenderCore)(m_device, vertexDesc, renderpassDesc, m_UIProgram, "UIPipeline");
        if (!m_UIPipeline)
        {
            return false;
        }

        m_UIPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_UIPipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_UIPipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_UIPipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
        m_UIPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_UIPipeline->setBlendEnable(true);
        m_UIPipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_SrcAlpha, renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha);
        m_UIPipeline->setColorBlendOp(renderer::BlendOperation::BlendOp_Add);
        m_UIPipeline->setAlphaBlendFactor(renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha, renderer::BlendFactor::BlendFactor_Zero);
        m_UIPipeline->setAlphaBlendOp(renderer::BlendOperation::BlendOp_Add);
        m_UIPipeline->setDepthWrite(false);
        m_UIPipeline->setDepthTest(false);
    }

    return true;
}

void ImGuiWigetHandler::destroyPipeline()
{
    if (m_UIPipeline)
    {
        V3D_DELETE(m_UIPipeline, memory::MemoryLabel::MemoryRenderCore);
        m_UIPipeline = nullptr;
    }

    if (m_UIProgram)
    {
        V3D_DELETE(m_UIProgram, memory::MemoryLabel::MemoryRenderCore);
        m_UIProgram = nullptr;
    }
}

bool ImGuiWigetHandler::renderDrawData(ImGuiWigetViewportData* viewportData, ImDrawData* imDrawData)
{
    const u32 currentIndex = m_frameCounter % k_countSwapchaints;

    s32 fbWidth = (s32)(imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
    s32 fbHeight = (s32)(imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0)
    {
        return false;
    }

    u32 vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    u32 indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    if (vertexBufferSize == 0 || indexBufferSize == 0)
    {
        return false;
    }

    u32 indexCount = viewportData->_indexBuffer[currentIndex]->getIndicesCount();
    u32 vertexCount = viewportData->_vertexBuffer[currentIndex]->getVerticesCount();
    if (imDrawData->TotalIdxCount > indexCount || imDrawData->TotalVtxCount > vertexCount)
    {
        destroyBuffers(viewportData);
        createBuffers(viewportData, std::max<u32>(indexCount, imDrawData->TotalIdxCount), std::max<u32>(vertexCount, imDrawData->TotalVtxCount));
    }
    ASSERT(indexBufferSize <= viewportData->_indexBuffer[currentIndex]->getIndicesCount() * sizeof(u16), "out of size");
    ASSERT(vertexBufferSize <= viewportData->_vertexBuffer[currentIndex]->getSize(), "out of size");

    ImDrawIdx* idxDst = viewportData->_indexBuffer[currentIndex]->map<ImDrawIdx>();
    ImDrawVert* vtxDst = viewportData->_vertexBuffer[currentIndex]->map<ImDrawVert>();
    for (u32 n = 0; n < imDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];

        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        idxDst += cmd_list->IdxBuffer.Size;
        vtxDst += cmd_list->VtxBuffer.Size;
    }

    viewportData->_indexBuffer[currentIndex]->unmap();
    viewportData->_vertexBuffer[currentIndex]->unmap();

    static auto setupRenderState = [this](renderer::CmdListRender* cmdList, ImDrawData* imDrawData, s32 fbWidth, s32 fbHeight) -> void
        {
            cmdList->setViewport(math::Rect32(0, 0, fbWidth, fbHeight));
            cmdList->setPipelineState(*m_UIPipeline);

            struct PushConstant
            {
                math::Vector2D _scale;
                math::Vector2D _translate;
            } pushConstBlock;

            pushConstBlock._scale = math::Vector2D(2.0f / imDrawData->DisplaySize.x, 2.0f / imDrawData->DisplaySize.y);
            pushConstBlock._translate = math::Vector2D(-1.0f - imDrawData->DisplayPos.x * pushConstBlock._scale.m_x, -1.0f - imDrawData->DisplayPos.y * pushConstBlock._scale.m_y);

            cmdList->bindPushConstant(renderer::ShaderType::Vertex, sizeof(PushConstant), &pushConstBlock);
        };

    if (imDrawData->CmdListsCount > 0)
    {
        setupRenderState(viewportData->_cmdList, imDrawData, fbWidth, fbHeight);

        s32 global_vtx_offset = 0;
        s32 global_idx_offset = 0;

        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_off = imDrawData->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = imDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        for (u32 i = 0; i < imDrawData->CmdListsCount; i++)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            for (u32 j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd* pCmd = &cmd_list->CmdBuffer[j];
                if (pCmd->UserCallback)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pCmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        setupRenderState(viewportData->_cmdList, imDrawData, fbWidth, fbHeight);
                    }
                    else
                    {
                        pCmd->UserCallback(cmd_list, pCmd);
                    }
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pCmd->ClipRect.x - clip_off.x) * clip_scale.x, (pCmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pCmd->ClipRect.z - clip_off.x) * clip_scale.x, (pCmd->ClipRect.w - clip_off.y) * clip_scale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                    if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                    if (clip_max.x > fbWidth) { clip_max.x = (f32)fbWidth; }
                    if (clip_max.y > fbHeight) { clip_max.y = (f32)fbHeight; }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    s32 x = (s32)(clip_min.x);
                    s32 y = (s32)(clip_min.y);
                    u32 width = (u32)(clip_max.x - clip_min.x);
                    u32 height = (u32)(clip_max.y - clip_min.y);

                    viewportData->_cmdList->setScissor(math::Rect32(x, y, x + width, y + height));

                    u64 textureID = pCmd->GetTexID();
                    ASSERT(textureID < m_activeTextures.size() && m_activeTextures[textureID], "range out or invalid");
                    viewportData->_cmdList->bindTexture(0, 0, m_activeTextures[textureID]);
                    viewportData->_cmdList->bindSampler(0, 1, *m_imageSampler);
                }

                viewportData->_cmdList->drawIndexed(viewportData->_geometryDesc[currentIndex], pCmd->IdxOffset + global_idx_offset, pCmd->ElemCount, pCmd->VtxOffset + global_vtx_offset, 0, 1);
            }

            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    return true;
}

bool ImGuiWigetHandler::render(renderer::CmdListRender* cmdList)
{
    ++m_frameCounter;

    ImGuiIO& imguiIO = ImGui::GetIO();
    if (imguiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::RenderPlatformWindowsDefault();
    }

    m_viewportData->_cmdList = cmdList;
    m_viewportData->_swapchain = nullptr;
    m_viewportData->_renderTarget = nullptr;

    ImDrawData* imDrawData = ImGui::GetDrawData();

    return renderDrawData(m_viewportData, imDrawData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI