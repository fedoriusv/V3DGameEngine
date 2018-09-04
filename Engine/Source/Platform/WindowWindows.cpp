#include "WindowWindows.h"
#include "Event/InputEventKeyboard.h"
#include "Event/InputEventMouse.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_WINDOWS
#include <winuser.h>

namespace v3d
{
namespace platform
{

WindowWindows::WindowWindows(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
    , m_hInstance(NULL)
    , m_hWnd(NULL)
{
    LOG_DEBUG("WindowWindows::WindowWindows: Created Windows window %llx", this);
    fillKeyCodes();
}

bool WindowWindows::initialize()
{
    m_hInstance = GetModuleHandle(NULL);

    // Register Class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wcex.lpfnWndProc = WindowWindows::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_classname.c_str();
    wcex.hIconSm = 0;
    if (!RegisterClassEx(&wcex))
    {
        ASSERT(false, "RegisterClassEx is failed");
        return false;
    }

    DWORD dwStyle = WS_POPUP;
    DWORD dwExStyle = WS_EX_APPWINDOW;

    if (!m_params._isFullscreen)
    {
        //dwStyle   = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;;
        dwExStyle = WS_EX_APPWINDOW;
    }

    RECT borderRect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&borderRect, dwStyle, FALSE, dwExStyle);

    // Border rect size is negative - see MoveWindowTo
    m_params._position.x += borderRect.left;
    m_params._position.y += borderRect.top;

    // Inflate the window size by the OS border

    core::Dimension2D size =
    {
        m_params._size.width + borderRect.right - borderRect.left,
        m_params._size.height + borderRect.bottom - borderRect.top
    };
    // create window
    ASSERT(!m_hWnd, "Already exist");
    m_hWnd = CreateWindowEx(dwExStyle, m_classname.c_str(), __TEXT("Test"), dwStyle, m_params._position.x, m_params._position.y, size.width, size.height, NULL, NULL, m_hInstance, this);
    if (!m_hWnd)
    {
        const u32 Error = GetLastError();
        DWORD NumHandles = 0;
        GetProcessHandleCount(GetCurrentProcess(), &NumHandles);

        LOG_ERROR( "WindowWindows::initialize: CreateWindowEx is failed, window %llx", this);
        return false;
    }

    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    LOG_DEBUG("WindowWindows::initialize: HWND created, window %llx, handle %llx", this, m_hWnd);
    return true;
}

bool WindowWindows::update()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    msg.message = WM_NULL;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }

     return true;
}

void WindowWindows::destroy()
{
    MSG msg;
    PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
    DestroyWindow(m_hWnd);
    m_hWnd = NULL;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    UnregisterClass(m_classname.c_str(), hInstance);
}

WindowWindows::~WindowWindows()
{
    LOG_DEBUG("WindowWindows::~WindowWindows");
    ASSERT(!m_hWnd, "Not destroyed");
}

void WindowWindows::minimize()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wndpl);

    wndpl.showCmd = SW_SHOWMINNOACTIVE;
    SetWindowPlacement(m_hWnd, &wndpl);

    m_params._isMinimized = true;
    m_params._isMaximized = false;
 }

void WindowWindows::maximize()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wndpl);

    wndpl.showCmd = SW_SHOWMAXIMIZED;
    SetWindowPlacement(m_hWnd, &wndpl);

    m_params._isMinimized = false;
    m_params._isMaximized = true;
}

void WindowWindows::restore()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wndpl);

    wndpl.showCmd = SW_SHOWNORMAL;
    SetWindowPlacement(m_hWnd, &wndpl);

    m_params._isMinimized = false;
    m_params._isMaximized = false;
}

void WindowWindows::setFullScreen(bool value)
{
    //TODO:
    ASSERT(false, "not implemented");
}

void WindowWindows::setResizeble(bool value)
{
    //TODO:
    ASSERT(false, "not implemented");
}

void WindowWindows::setTextCaption(const std::string& text)
{
    SetWindowTextA(m_hWnd, text.c_str());
}

void WindowWindows::setPosition(const core::Point2D& pos)
{
    if (m_params._isFullscreen)
    {
        return;
    }

    SetWindowPos(m_hWnd, NULL, pos.x, pos.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    m_params._position = pos;
}

bool WindowWindows::isMaximized() const
{
    return m_params._isMaximized;
}

bool WindowWindows::isMinimized() const
{
    return  m_params._isMinimized;
}

bool WindowWindows::isActive() const
{
    if (m_hWnd == GetActiveWindow())
    {
        return m_params._isActive;
    }

    return false;
}

bool WindowWindows::isFocused() const
{
    if (m_hWnd == GetFocus())
    {
        return m_params._isFocused;
    }

    return false;
}

NativeInstance WindowWindows::getInstance() const
{
    return m_hInstance;
}

NativeWindows WindowWindows::getWindowHandle() const
{
    return m_hWnd;
}

LRESULT WindowWindows::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_receiver)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    auto getModifiers = [](WPARAM wParam, LPARAM lParam) -> u16
    {
        u16 modifers = 0;

        switch (wParam)
        {
        case VK_MENU: //Alt
            if ((lParam & 0x1000000) == 0) //left
            {
                modifers |= event::KeyModifierCode::KeyModifier_Alt;
            }
            else //right
            {
                modifers |= event::KeyModifierCode::KeyModifier_Alt;
            }
            break;

        case VK_CONTROL:
            if ((lParam & 0x1000000) == 0) //left
            {
                modifers |= event::KeyModifierCode::KeyModifier_Ctrl;
            }
            else //right
            {
                modifers |= event::KeyModifierCode::KeyModifier_Ctrl;
            }
            break;

        case VK_SHIFT:
        {
            s32 actualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (actualKey == VK_LSHIFT)
            {
                modifers |= event::KeyModifierCode::KeyModifier_Shift;
            }
            else
            {
                modifers |= event::KeyModifierCode::KeyModifier_Shift;
            }
            break;
        }

        case VK_CAPITAL:
            modifers |= event::KeyModifierCode::KeyModifier_CapsLock;
            break;

        }

        return modifers;
    };

    switch (message)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        event::KeyboardInputEvent* event = new(m_receiver->allocateInputEvent()) event::KeyboardInputEvent();
        event->_event = event::KeyboardInputEvent::KeyboardPressDown;
        event->_key = m_keyCodes.get((u32)wParam);
        event->_character = (c8)wParam;
        event->_modifers = getModifiers(wParam, lParam);

        m_receiver->pushEvent(event);
        return TRUE;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        event::KeyboardInputEvent* event = new(m_receiver->allocateInputEvent()) event::KeyboardInputEvent();
        event->_event = event::KeyboardInputEvent::KeyboardPressUp;
        event->_key = m_keyCodes.get((u32)wParam);
        event->_character = (c8)wParam;
        event->_modifers = getModifiers(wParam, lParam);

        m_receiver->pushEvent(event);
        return TRUE;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    {
        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)LOWORD(lParam);
        event->_cursorPosition.y = (s16)HIWORD(lParam);
        event->_wheelValue = ((f32)((s16)HIWORD(wParam))) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MousePressDown;
        event->_modifers = 0;

        switch (message)
        {
        case WM_LBUTTONDOWN:
            event->_key = event::KeyCode::KeyLButton;
            break;

        case WM_RBUTTONDOWN:
            event->_key = event::KeyCode::KeyRButton;
            break;

        case WM_MBUTTONDOWN:
            event->_key = event::KeyCode::KeyMButton;
            break;

        case WM_XBUTTONDOWN:
            event->_key = event::KeyCode::KeyXButton1;
            break;
        }
        m_receiver->pushEvent(event);
        return TRUE;
    }


    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)LOWORD(lParam);
        event->_cursorPosition.y = (s16)HIWORD(lParam);
        event->_wheelValue = ((f32)((s16)HIWORD(wParam))) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MousePressUp;
        event->_modifers = 0;

        switch (message)
        {
        case WM_LBUTTONUP:
            event->_key = event::KeyCode::KeyLButton;
            break;

        case WM_RBUTTONUP:
            event->_key = event::KeyCode::KeyRButton;
            break;

        case WM_MBUTTONUP:
            event->_key = event::KeyCode::KeyMButton;
            break;

        case WM_XBUTTONUP:
            event->_key = event::KeyCode::KeyXButton1;
            break;
        }
        m_receiver->pushEvent(event);
        return TRUE;
    }

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    {
        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)LOWORD(lParam);
        event->_cursorPosition.y = (s16)HIWORD(lParam);
        event->_wheelValue = ((f32)((s16)HIWORD(wParam))) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseDoubleClick;
        event->_modifers = 0;

        switch (message)
        {
        case WM_LBUTTONDBLCLK:
            event->_key = event::KeyCode::KeyLButton;
            break;

        case WM_RBUTTONDBLCLK:
            event->_key = event::KeyCode::KeyRButton;
            break;

        case WM_MBUTTONDBLCLK:
            event->_key = event::KeyCode::KeyMButton;
            break;

        case WM_XBUTTONDBLCLK:
            event->_key = event::KeyCode::KeyXButton1;
            break;
        }
        m_receiver->pushEvent(event);
        return TRUE;
    }

    case WM_MOUSEWHEEL:
    {
        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)LOWORD(lParam);
        event->_cursorPosition.y = (s16)HIWORD(lParam);
        event->_wheelValue = ((f32)((s16)HIWORD(wParam))) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseWheel;
        event->_modifers = 0;

        m_receiver->pushEvent(event);
        return TRUE;
    }

    case WM_MOUSEMOVE:
    {
        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)LOWORD(lParam);
        event->_cursorPosition.y = (s16)HIWORD(lParam);
        event->_wheelValue = ((f32)((s16)HIWORD(wParam))) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseMoved;
        event->_modifers = 0;

        m_receiver->pushEvent(event);
        return TRUE;
    }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WindowWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static u32 s_timer_ID = 0;

    if (message == WM_CREATE)
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);

        SetTimer(hWnd, s_timer_ID, 1000, NULL); //milliseconds

        return TRUE;
    }
    else
    {
        LONG_PTR lpCreateParams = 0;
        lpCreateParams = GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (!lpCreateParams)
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        WindowWindows* window = reinterpret_cast<WindowWindows*>(lpCreateParams);

        switch (message)
        {
        case WM_ACTIVATE:
        {
            BOOL active = LOWORD(wParam) != WA_INACTIVE;
            BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

            if (active && iconified)
            {
                active = FALSE;
            }
            window->m_params._isActive = active;


            return TRUE;
        }

        case WM_SETFOCUS:
        {
            window->m_params._isFocused = true;
            return TRUE;
        }

        case WM_KILLFOCUS:
        {
            window->m_params._isFocused = false;
            return TRUE;
        }

        case WM_SHOWWINDOW:
        {
            return TRUE;
        }

        case WM_SIZE:
        {
            return TRUE;
        }

        case WM_TIMER:
        {
            return TRUE;
        }

        case WM_DESTROY:
        {
            KillTimer(hWnd, s_timer_ID);
            PostQuitMessage(0);
            return TRUE;
        }

        default:
        {
            ASSERT(window, "Must be valid pointer");
            return window->HandleMessage(hWnd, message, wParam, lParam);
        }
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void WindowWindows::fillKeyCodes()
{
    m_keyCodes.add(event::KeyCode::KeyUknown, 0x00);
    m_keyCodes.add(event::KeyCode::KeyLButton, 0x01);
    m_keyCodes.add(event::KeyCode::KeyRButton, 0x02);
    m_keyCodes.add(event::KeyCode::KeyCancel, 0x03);
    m_keyCodes.add(event::KeyCode::KeyMButton, 0x04);
    m_keyCodes.add(event::KeyCode::KeyXButton1, 0x05);
    m_keyCodes.add(event::KeyCode::KeyXButton2, 0x06);
    m_keyCodes.add(event::KeyCode::KeyBack, 0x08);
    m_keyCodes.add(event::KeyCode::KeyTab, 0x09);
    m_keyCodes.add(event::KeyCode::KeyClear, 0x0C);
    m_keyCodes.add(event::KeyCode::KeyReturn, 0x0D);
    m_keyCodes.add(event::KeyCode::KeyShift, 0x10);
    m_keyCodes.add(event::KeyCode::KeyControl, 0x11);
    m_keyCodes.add(event::KeyCode::KeyMenu, 0x12);
    m_keyCodes.add(event::KeyCode::KeyPause, 0x13);
    m_keyCodes.add(event::KeyCode::KeyCapital, 0x14);
    m_keyCodes.add(event::KeyCode::KeyKana, 0x15);
    m_keyCodes.add(event::KeyCode::KeyHanguel, 0x15);
    m_keyCodes.add(event::KeyCode::KeyHangul, 0x15);
    m_keyCodes.add(event::KeyCode::KeyJunja, 0x17);
    m_keyCodes.add(event::KeyCode::KeyFinal, 0x18);
    m_keyCodes.add(event::KeyCode::KeyHanja, 0x19);
    m_keyCodes.add(event::KeyCode::KeyKanji, 0x19);
    m_keyCodes.add(event::KeyCode::KeyEscape, 0x1B);
    m_keyCodes.add(event::KeyCode::KeyConvert, 0x1C);
    m_keyCodes.add(event::KeyCode::KeyNonconvert, 0x1D);
    m_keyCodes.add(event::KeyCode::KeyAccept, 0x1E);
    m_keyCodes.add(event::KeyCode::KeyModechange, 0x1F);
    m_keyCodes.add(event::KeyCode::KeySpace, 0x20);
    m_keyCodes.add(event::KeyCode::KeyPrior, 0x21);
    m_keyCodes.add(event::KeyCode::KeyNext, 0x22);
    m_keyCodes.add(event::KeyCode::KeyEnd, 0x23);
    m_keyCodes.add(event::KeyCode::KeyHome, 0x24);
    m_keyCodes.add(event::KeyCode::KeyLeft, 0x25);
    m_keyCodes.add(event::KeyCode::KeyUp, 0x26);
    m_keyCodes.add(event::KeyCode::KeyRight, 0x27);
    m_keyCodes.add(event::KeyCode::KeyDown, 0x28);
    m_keyCodes.add(event::KeyCode::KeySelect, 0x29);
    m_keyCodes.add(event::KeyCode::KeyPrint, 0x2A);
    m_keyCodes.add(event::KeyCode::KeyExecut, 0x2B);
    m_keyCodes.add(event::KeyCode::KeySnapshot, 0x2C);
    m_keyCodes.add(event::KeyCode::KeyInsert, 0x2D);
    m_keyCodes.add(event::KeyCode::KeyDelete, 0x2E);
    m_keyCodes.add(event::KeyCode::KeyHelp, 0x2F);
    m_keyCodes.add(event::KeyCode::KeyKey_0, 0x30);
    m_keyCodes.add(event::KeyCode::KeyKey_1, 0x31);
    m_keyCodes.add(event::KeyCode::KeyKey_2, 0x32);
    m_keyCodes.add(event::KeyCode::KeyKey_3, 0x33);
    m_keyCodes.add(event::KeyCode::KeyKey_4, 0x34);
    m_keyCodes.add(event::KeyCode::KeyKey_5, 0x35);
    m_keyCodes.add(event::KeyCode::KeyKey_6, 0x36);
    m_keyCodes.add(event::KeyCode::KeyKey_7, 0x37);
    m_keyCodes.add(event::KeyCode::KeyKey_8, 0x38);
    m_keyCodes.add(event::KeyCode::KeyKey_9, 0x39);
    m_keyCodes.add(event::KeyCode::KeyKey_A, 0x41);
    m_keyCodes.add(event::KeyCode::KeyKey_B, 0x42);
    m_keyCodes.add(event::KeyCode::KeyKey_C, 0x43);
    m_keyCodes.add(event::KeyCode::KeyKey_D, 0x44);
    m_keyCodes.add(event::KeyCode::KeyKey_E, 0x45);
    m_keyCodes.add(event::KeyCode::KeyKey_F, 0x46);
    m_keyCodes.add(event::KeyCode::KeyKey_G, 0x47);
    m_keyCodes.add(event::KeyCode::KeyKey_H, 0x48);
    m_keyCodes.add(event::KeyCode::KeyKey_I, 0x49);
    m_keyCodes.add(event::KeyCode::KeyKey_J, 0x4A);
    m_keyCodes.add(event::KeyCode::KeyKey_K, 0x4B);
    m_keyCodes.add(event::KeyCode::KeyKey_L, 0x4C);
    m_keyCodes.add(event::KeyCode::KeyKey_M, 0x4D);
    m_keyCodes.add(event::KeyCode::KeyKey_N, 0x4E);
    m_keyCodes.add(event::KeyCode::KeyKey_O, 0x4F);
    m_keyCodes.add(event::KeyCode::KeyKey_P, 0x50);
    m_keyCodes.add(event::KeyCode::KeyKey_Q, 0x51);
    m_keyCodes.add(event::KeyCode::KeyKey_R, 0x52);
    m_keyCodes.add(event::KeyCode::KeyKey_S, 0x53);
    m_keyCodes.add(event::KeyCode::KeyKey_T, 0x54);
    m_keyCodes.add(event::KeyCode::KeyKey_U, 0x55);
    m_keyCodes.add(event::KeyCode::KeyKey_V, 0x56);
    m_keyCodes.add(event::KeyCode::KeyKey_W, 0x57);
    m_keyCodes.add(event::KeyCode::KeyKey_X, 0x58);
    m_keyCodes.add(event::KeyCode::KeyKey_Y, 0x59);
    m_keyCodes.add(event::KeyCode::KeyKey_Z, 0x5A);
    m_keyCodes.add(event::KeyCode::KeyLWin, 0x5B);
    m_keyCodes.add(event::KeyCode::KeyRWin, 0x5C);
    m_keyCodes.add(event::KeyCode::KeyApps, 0x5D);
    m_keyCodes.add(event::KeyCode::KeySleep, 0x5F);
    m_keyCodes.add(event::KeyCode::KeyNumpad0, 0x60);
    m_keyCodes.add(event::KeyCode::KeyNumpad1, 0x61);
    m_keyCodes.add(event::KeyCode::KeyNumpad2, 0x62);
    m_keyCodes.add(event::KeyCode::KeyNumpad3, 0x63);
    m_keyCodes.add(event::KeyCode::KeyNumpad4, 0x64);
    m_keyCodes.add(event::KeyCode::KeyNumpad5, 0x65);
    m_keyCodes.add(event::KeyCode::KeyNumpad6, 0x66);
    m_keyCodes.add(event::KeyCode::KeyNumpad7, 0x67);
    m_keyCodes.add(event::KeyCode::KeyNumpad8, 0x68);
    m_keyCodes.add(event::KeyCode::KeyNumpad9, 0x69);
    m_keyCodes.add(event::KeyCode::KeyMultiply, 0x6A);
    m_keyCodes.add(event::KeyCode::KeyAdd, 0x6B);
    m_keyCodes.add(event::KeyCode::KeySeparator, 0x6C);
    m_keyCodes.add(event::KeyCode::KeySubtract, 0x6D);
    m_keyCodes.add(event::KeyCode::KeyDecimal, 0x6E);
    m_keyCodes.add(event::KeyCode::KeyDivide, 0x6F);
    m_keyCodes.add(event::KeyCode::KeyF1, 0x70);
    m_keyCodes.add(event::KeyCode::KeyF2, 0x71);
    m_keyCodes.add(event::KeyCode::KeyF3, 0x72);
    m_keyCodes.add(event::KeyCode::KeyF4, 0x73);
    m_keyCodes.add(event::KeyCode::KeyF5, 0x74);
    m_keyCodes.add(event::KeyCode::KeyF6, 0x75);
    m_keyCodes.add(event::KeyCode::KeyF7, 0x76);
    m_keyCodes.add(event::KeyCode::KeyF8, 0x77);
    m_keyCodes.add(event::KeyCode::KeyF9, 0x78);
    m_keyCodes.add(event::KeyCode::KeyF10, 0x79);
    m_keyCodes.add(event::KeyCode::KeyF11, 0x7A);
    m_keyCodes.add(event::KeyCode::KeyF12, 0x7B);
    m_keyCodes.add(event::KeyCode::KeyF13, 0x7C);
    m_keyCodes.add(event::KeyCode::KeyF14, 0x7D);
    m_keyCodes.add(event::KeyCode::KeyF15, 0x7E);
    m_keyCodes.add(event::KeyCode::KeyF16, 0x7F);
    m_keyCodes.add(event::KeyCode::KeyF17, 0x80);
    m_keyCodes.add(event::KeyCode::KeyF18, 0x81);
    m_keyCodes.add(event::KeyCode::KeyF19, 0x82);
    m_keyCodes.add(event::KeyCode::KeyF20, 0x83);
    m_keyCodes.add(event::KeyCode::KeyF21, 0x84);
    m_keyCodes.add(event::KeyCode::KeyF22, 0x85);
    m_keyCodes.add(event::KeyCode::KeyF23, 0x86);
    m_keyCodes.add(event::KeyCode::KeyF24, 0x87);
    m_keyCodes.add(event::KeyCode::KeyNumlock, 0x90);
    m_keyCodes.add(event::KeyCode::KeyScroll, 0x91);
    m_keyCodes.add(event::KeyCode::KeyLShift, 0xA0);
    m_keyCodes.add(event::KeyCode::KeyRShift, 0xA1);
    m_keyCodes.add(event::KeyCode::KeyLControl, 0xA2);
    m_keyCodes.add(event::KeyCode::KeyRControl, 0xA3);
    m_keyCodes.add(event::KeyCode::KeyLMenu, 0xA4);
    m_keyCodes.add(event::KeyCode::KeyRMenu, 0xA5);
    m_keyCodes.add(event::KeyCode::KeyPlus, 0xBB);
    m_keyCodes.add(event::KeyCode::KeyComma, 0xBC);
    m_keyCodes.add(event::KeyCode::KeyMinus, 0xBD);
    m_keyCodes.add(event::KeyCode::KeyPeriod, 0xBE);
    m_keyCodes.add(event::KeyCode::KeyAttn, 0xF6);
    m_keyCodes.add(event::KeyCode::KeyCrSel, 0xF7);
    m_keyCodes.add(event::KeyCode::KeyExSel, 0xF8);
    m_keyCodes.add(event::KeyCode::KeyErEof, 0xF9);
    m_keyCodes.add(event::KeyCode::KeyPlay, 0xFA);
    m_keyCodes.add(event::KeyCode::KeyZoom, 0xFB);
    m_keyCodes.add(event::KeyCode::KeyPA1, 0xFD);
    m_keyCodes.add(event::KeyCode::KeyOem_Clear, 0xFE);
}

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
