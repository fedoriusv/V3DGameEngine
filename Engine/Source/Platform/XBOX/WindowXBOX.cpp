#include "WindowXBOX.h"
#include "Event/InputEventKeyboard.h"
#include "Event/InputEventMouse.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_XBOX
#include <XGameRuntimeInit.h>
#include <XSystem.h>
#include <XGameErr.h>
#include <appnotify.h>

namespace v3d
{
namespace platform
{

HANDLE WindowXBOX::s_suspendComplete = NULL;
HANDLE WindowXBOX::s_signalResume = NULL;

WindowXBOX::WindowXBOX(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
    , m_hInstance(NULL)
    , m_hWnd(NULL)

    , m_hPLM(NULL)
{
    LOG_DEBUG("WindowXBOX::WindowXBOX: Created Windows window %llx", this);
}

bool WindowXBOX::initialize()
{
    HRESULT result = XGameRuntimeInitialize();
    if (FAILED(result))
    {
        if (result == E_GAMERUNTIME_DLL_NOT_FOUND || result == E_GAMERUNTIME_VERSION_MISMATCH)
        {
            LOG_ERROR("WindowXBOX::initialize : Game Runtime is not installed on this system or needs updating");
        }

        ASSERT(false, "XGameRuntimeInitialize is failed");
        return false;
    }

    // Default main thread to CPU 0
    SetThreadAffinityMask(GetCurrentThread(), 0x1);

    m_hInstance = GetModuleHandle(NULL);
    ASSERT(m_hInstance, "m_hInstance must be valid");

    // Register Class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowXBOX::WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.lpszClassName = m_classname.c_str();
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassExW(&wcex))
    {
        //https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
        const u32 error = GetLastError();

        ASSERT(false, "RegisterClassExW is failed");
        return false;
    }

    // Create a window.
    RECT rect = { 0, 0, static_cast<LONG>(m_params._size.width), static_cast<LONG>(m_params._size.height) };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowExW(0, m_classname.c_str(), __TEXT("XBOX"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_hInstance, nullptr);
    if (!m_hWnd)
    {
        const u32 error = GetLastError();

        LOG_ERROR("WindowXBOX::initialize: CreateWindowExW is failed. Error %d, window %llx", error, this);
        return false;
    }

    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    s_suspendComplete = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    s_signalResume = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (!s_suspendComplete || !s_signalResume)
    {
        ASSERT(false, "Can't create events");
        return false;
    }

    if (RegisterAppStateChangeNotification([](BOOLEAN quiesced, PVOID context) -> void
        {
            if (quiesced)
            {
                ResetEvent(WindowXBOX::s_suspendComplete);
                ResetEvent(WindowXBOX::s_signalResume);

                // To ensure we use the main UI thread to process the notification, we self-post a message
                PostMessage(reinterpret_cast<HWND>(context), WM_USER, 0, 0);

                // To defer suspend, you must wait to exit this callback
                WaitForSingleObject(WindowXBOX::s_suspendComplete, INFINITE);
            }
            else
            {
                SetEvent(WindowXBOX::s_signalResume);
            }
        }, m_hWnd, &m_hPLM))
    {
        ASSERT(false, "Can't register events");
        return false;
    }

    XSystemDeviceType type = XSystemGetDeviceType();
    XSystemAnalyticsInfo info = XSystemGetAnalyticsInfo();
    ASSERT(type == XSystemDeviceType::XboxScarlettAnaconda || type == XSystemDeviceType::XboxScarlettLockhart, "Must be xbox series");

    LOG_DEBUG("WindowXBOX::initialize: HWND created, window %llx, handle %llx", this, m_hWnd);
    return true;
}

bool WindowXBOX::update()
{
    MSG msg = {};
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

void WindowXBOX::destroy()
{
    if (m_hPLM)
    {
        UnregisterAppStateChangeNotification(m_hPLM);
        m_hPLM = NULL;
    }

    CloseHandle(s_suspendComplete);
    s_suspendComplete = NULL;

    CloseHandle(s_signalResume);
    s_signalResume = NULL;

    if (m_hWnd)
    {
        MSG msg = {};
        PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    XGameRuntimeUninitialize();

    HINSTANCE hInstance = GetModuleHandle(NULL);
    UnregisterClass(m_classname.c_str(), hInstance);
    hInstance = NULL;
}

WindowXBOX::~WindowXBOX()
{
    LOG_DEBUG("WindowWindows::~WindowWindows");

    ASSERT(!s_suspendComplete, "Not destroyed");
    ASSERT(!s_signalResume, "Not destroyed");

    ASSERT(!m_hPLM, "Not unregistered");
    ASSERT(!m_hWnd, "Not destroyed");
}

void WindowXBOX::minimize()
{
    m_params._isMinimized = true;
    m_params._isMaximized = false;

    //TODO suspend
}

void WindowXBOX::maximize()
{
    m_params._isMinimized = false;
    m_params._isMaximized = true;

    //TODO resume
}

void WindowXBOX::restore()
{
}

void WindowXBOX::setFullScreen(bool value)
{
    //is not used
}

void WindowXBOX::setResizeble(bool value)
{
    //is not used
}

void WindowXBOX::setTextCaption(const std::string& text)
{
    //is not used
}

void WindowXBOX::setPosition(const core::Point2D& pos)
{
    //is not used
}

bool WindowXBOX::isMaximized() const
{
    return m_params._isMaximized;
}

bool WindowXBOX::isMinimized() const
{
    return  m_params._isMinimized;
}

bool WindowXBOX::isActive() const
{
    return m_params._isActive;
}

bool WindowXBOX::isFocused() const
{
    return m_params._isFocused;
}

NativeInstance WindowXBOX::getInstance() const
{
    return m_hInstance;
}

NativeWindows WindowXBOX::getWindowHandle() const
{
    return m_hWnd;
}

bool WindowXBOX::isValid() const
{
    return m_hWnd != NULL;
}

LRESULT WindowXBOX::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR lpCreateParams = 0;
    lpCreateParams = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!lpCreateParams)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    WindowXBOX* window = reinterpret_cast<WindowXBOX*>(lpCreateParams);

    switch (message)
    {
    case WM_CREATE:
    {
        return TRUE;
    }

    case WM_ACTIVATEAPP:
    case WM_ACTIVATE:
    {
        BOOL active = LOWORD(wParam) != WA_INACTIVE;
        BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

        if (active && iconified)
        {
            active = FALSE;
        }
        ASSERT(window, "Must be valid pointer");
        window->m_params._isActive = active;

        return TRUE;
    }

    case WM_SETFOCUS:
    {
        ASSERT(window, "Must be valid pointer");
        window->m_params._isFocused = true;

        return TRUE;
    }

    case WM_KILLFOCUS:
    {
        ASSERT(window, "Must be valid pointer");
        window->m_params._isFocused = false;

        return TRUE;
    }

    case WM_SHOWWINDOW:
    {
        return TRUE;
    }

    case WM_USER:
    {
        ASSERT(window, "Must be valid pointer");
        window->minimize();

       // Complete deferral
       SetEvent(WindowXBOX::s_suspendComplete);
       (void)WaitForSingleObject(WindowXBOX::s_signalResume, INFINITE);

      window->maximize();

      return TRUE;
    }

    default:
    {
        ASSERT(window, "Must be valid pointer");
        return window->ReceiverMessage(hWnd, message, wParam, lParam);
    }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WindowXBOX::ReceiverMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_receiver)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    auto getModifiers = [](WPARAM wParam, LPARAM lParam) -> u8
    {
        u8 modifers = 0;

        switch (wParam)
        {
        case VK_MENU: //Alt
            modifers |= event::KeyModifierCode::KeyModifier_Alt;
            break;

        case VK_CONTROL:
            modifers |= event::KeyModifierCode::KeyModifier_Ctrl;
            break;

        case VK_SHIFT:
            modifers |= event::KeyModifierCode::KeyModifier_Shift;
            break;

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
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);

        event::MouseInputEvent* event = new(m_receiver->allocateInputEvent()) event::MouseInputEvent();
        event->_cursorPosition.x = (s16)pt.x;
        event->_cursorPosition.y = (s16)pt.y;
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

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
