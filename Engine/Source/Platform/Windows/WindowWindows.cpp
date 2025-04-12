#include "Events/InputEventKeyboard.h"
#include "Events/InputEventMouse.h"
#include "Events/InputEventReceiver.h"
#include "Events/SystemEvent.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "WindowWindows.h"

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#include <windows.h>
#include <windowsx.h>

namespace v3d
{
namespace platform
{

WindowWindows::WindowWindows(const WindowParams& params, event::InputEventReceiver* receiver, Window* parent) noexcept
    : Window(params, receiver)
    , m_hInstance(NULL)
    , m_hWnd(NULL)
    , m_parent(parent)
    , m_className(L"Window")
{
    LOG_DEBUG("WindowWindows::WindowWindows: Created Windows window %llx", this);
    fillKeyCodes();
}

bool WindowWindows::initialize()
{
    m_hInstance = GetModuleHandle(NULL);
    std::wstring className = m_className;
    HWND parentHWnd = nullptr;

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
    DWORD dwExStyle = WS_EX_APPWINDOW;

    if (m_parent)
    {
        dwStyle = WS_POPUP;
        dwExStyle = WS_EX_APPWINDOW;

        parentHWnd = m_parent->getWindowHandle();

        WindowWindows* parent = static_cast<WindowWindows*>(m_parent);
        parent->m_children.push_back(this);

        m_className = parent->m_className;
    }
    else
    {
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
        wcex.lpszClassName = m_className.c_str();
        wcex.hIconSm = 0;
        if (!RegisterClassEx(&wcex))
        {
            ASSERT(false, "RegisterClassEx is failed");
            return false;
        }

        if (m_params._isFullscreen)
        {
            dwStyle = WS_POPUP | WS_VISIBLE;
            dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
        }
        else if (m_params._isResizable)
        {
            dwStyle |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        }
    }

    RECT borderRect = { 
        m_params._position.m_x, 
        m_params._position.m_y, 
        m_params._position.m_x + (s32)m_params._size.m_width,
        m_params._position.m_y + (s32)m_params._size.m_height
    };
    AdjustWindowRectEx(&borderRect, dwStyle, FALSE, dwExStyle);

    // create window
    ASSERT(!m_hWnd, "Already exist");
    m_hWnd = CreateWindowEx(dwExStyle, className.c_str(), m_params._text.c_str(), dwStyle,
        borderRect.left, borderRect.top, borderRect.right - borderRect.left, borderRect.bottom - borderRect.top, parentHWnd, NULL, m_hInstance, this);

    if (!m_hWnd)
    {
        const u32 error = GetLastError();
        DWORD numHandles = 0;
        GetProcessHandleCount(GetCurrentProcess(), &numHandles);

        //https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes
        LOG_ERROR( "WindowWindows::initialize: CreateWindowEx is failed, error %u, window %llx", error, this);
        return false;
    }

    if (m_params._isFullscreen)
    {
        ShowWindow(m_hWnd, SW_MAXIMIZE);
    }
    else
    {
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
    }
    UpdateWindow(m_hWnd);

#if !USE_LOGGER
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif //USE_LOGGER
    LOG_DEBUG("WindowWindows::initialize: HWND created, window %llx, handle %llx", this, m_hWnd);
    return true;
}

bool WindowWindows::update()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    msg.message = WM_NULL;

    HWND hWnd = m_children.empty() ? m_hWnd : nullptr;
    while(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     return true;
}

void WindowWindows::destroy()
{
    WindowReport report;
    report._flags = WindowReport::DestroyWindow;

    notify(report);


    if (m_parent && GetCapture() == m_hWnd)
    {
        ReleaseCapture();
        SetCapture(m_parent->getWindowHandle());

        WindowWindows* parent = static_cast<WindowWindows*>(m_parent);
        auto found = std::find(parent->m_children.begin(), parent->m_children.end(), this);
        ASSERT(found != parent->m_children.end(), "must be found");
        parent->m_children.erase(found);
    }

    DestroyWindow(m_hWnd);
    m_hWnd = NULL;

    if (!m_parent)
    {
        UnregisterClass(m_className.c_str(), m_hInstance);
    }
}

WindowWindows::~WindowWindows()
{
    LOG_DEBUG("WindowWindows::~WindowWindows");
    ASSERT(!m_hWnd, "Not destroyed");
    ASSERT(m_children.empty(), "must be empty");
}

void WindowWindows::minimize()
{
    ASSERT(m_hWnd, "Must be valid");

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
    ASSERT(m_hWnd, "Must be valid");

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
    ASSERT(m_hWnd, "Must be valid");

    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wndpl);

    wndpl.showCmd = SW_SHOWNORMAL;
    SetWindowPlacement(m_hWnd, &wndpl);

    m_params._isMinimized = false;
    m_params._isMaximized = false;
}

void WindowWindows::show()
{
    ASSERT(m_hWnd, "Must be valid");

    if (m_parent)
    {
        SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)nullptr);
    }

    if (m_params._isFocused)
    {
        ShowWindow(m_hWnd, SW_SHOWNA);
    }
    else
    {
        ShowWindow(m_hWnd, SW_SHOW);
    }

    if (m_parent)
    {
        SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)m_parent->getWindowHandle());
    }
}

void WindowWindows::focus()
{
    ASSERT(m_hWnd, "Must be valid");

    BringWindowToTop(m_hWnd);
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
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

void WindowWindows::setText(const std::wstring& text)
{
    ASSERT(m_hWnd, "Must be valid");

    SetWindowTextW(m_hWnd, text.c_str());
    DefWindowProcW(m_hWnd, WM_SETTEXT, 0, (LPARAM)text.c_str());
}

void WindowWindows::setSize(const math::Dimension2D& size)
{
    ASSERT(m_hWnd, "Must be valid");

    RECT rect = { 0, 0, (LONG)size.m_width, (LONG)size.m_height };
    DWORD dwStyle = ::GetWindowLongW(m_hWnd, GWL_STYLE);
    DWORD dwExStyle = ::GetWindowLongW(m_hWnd, GWL_EXSTYLE);

    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    SetWindowPos(m_hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void WindowWindows::setPosition(const math::Point2D& pos)
{
    ASSERT(m_hWnd, "Must be valid");

    if (m_params._isFullscreen)
    {
        return;
    }

    RECT rect = { (LONG)pos.m_x, (LONG)pos.m_y, (LONG)pos.m_x, (LONG)pos.m_y };
    DWORD dwStyle = ::GetWindowLongW(m_hWnd, GWL_STYLE);
    DWORD dwExStyle = ::GetWindowLongW(m_hWnd, GWL_EXSTYLE);

    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    SetWindowPos(m_hWnd, NULL, pos.m_x, pos.m_y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

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
    ASSERT(m_hWnd, "Must be valid");

    HWND hWnd = GetActiveWindow();
    m_params._isActive = m_hWnd == hWnd ? true : false;

    return m_params._isActive;
}

bool WindowWindows::isFocused() const
{
    ASSERT(m_hWnd, "Must be valid");

    HWND hWnd = GetForegroundWindow();
    m_params._isFocused = m_hWnd == hWnd ? true : false;

    return m_params._isFocused;
}

const math::Dimension2D& WindowWindows::getSize() const
{
    ASSERT(m_hWnd, "Must be valid");

    RECT rect;
    GetClientRect(m_hWnd, &rect);
    m_params._size = { (u32)(rect.right - rect.left), (u32)(rect.bottom - rect.top) };

    return m_params._size;
}

const math::Point2D& WindowWindows::getPosition() const
{
    ASSERT(m_hWnd, "Must be valid");

    POINT pos = { 0, 0 };
    ClientToScreen(m_hWnd, &pos);
    m_params._position = { pos.x, pos.y };

    return m_params._position;
}

NativeInstance WindowWindows::getInstance() const
{
    return m_hInstance;
}

NativeWindow WindowWindows::getWindowHandle() const
{
    return m_hWnd;
}

bool WindowWindows::isValid() const
{
    return m_hWnd != NULL;
}

LRESULT WindowWindows::HandleInputMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_receiver)
    {
        return DefWindowProc(m_hWnd, message, wParam, lParam);
    }

    static auto getModifiers = [](WPARAM wParam, LPARAM lParam) -> u8
    {
        u8 modifers = 0;

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
        event::KeyboardInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::KeyboardInputEvent());
        event->_event = event::KeyboardInputEvent::KeyboardPressDown;
        event->_key = m_keyCodes.get((u32)wParam);
        event->_character = (c8)wParam;
        event->_modifers = getModifiers(wParam, lParam);
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        event::KeyboardInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::KeyboardInputEvent());
        event->_event = event::KeyboardInputEvent::KeyboardPressUp;
        event->_key = m_keyCodes.get((u32)wParam);
        event->_character = (c8)wParam;
        event->_modifers = getModifiers(wParam, lParam);
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    {
        if (message == WM_LBUTTONDBLCLK && GetCapture() == nullptr)
        {
            SetCapture(m_hWnd); // Allow us to read mouse coordinates when dragging mouse outside of our window bounds.
        }

        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = GET_X_LPARAM(lParam);
        event->_clientCoordinates.m_y = GET_Y_LPARAM(lParam);
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseDoubleClick;
        event->_modifers = 0;
        event->_windowID = this->ID();

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
            event->_key = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? event::KeyCode::KeyXButton1 : event::KeyCode::KeyXButton2;
            break;
        }
        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    {
        if (message == WM_LBUTTONDOWN && GetCapture() == nullptr)
        {
            SetCapture(m_hWnd); // Allow us to read mouse coordinates when dragging mouse outside of our window bounds.
        }

        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = GET_X_LPARAM(lParam);
        event->_clientCoordinates.m_y = GET_Y_LPARAM(lParam);
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MousePressDown;
        event->_modifers = 0;
        event->_windowID = this->ID();

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
            event->_key = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? event::KeyCode::KeyXButton1 : event::KeyCode::KeyXButton2;
            break;
        }
        m_receiver->pushEvent(event);

        return FALSE;
    }


    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        if (message == WM_LBUTTONUP && GetCapture() == m_hWnd)
        {
            ReleaseCapture();
        }

        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = GET_X_LPARAM(lParam);
        event->_clientCoordinates.m_y = GET_Y_LPARAM(lParam);
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MousePressUp;
        event->_modifers = 0;
        event->_windowID = this->ID();

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
            event->_key = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? event::KeyCode::KeyXButton1 : event::KeyCode::KeyXButton2;
            break;
        }
        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_MOUSEWHEEL:
    {
        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = GET_X_LPARAM(lParam);
        event->_clientCoordinates.m_y = GET_Y_LPARAM(lParam);
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseWheel;
        event->_modifers = 0;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
    {
        //const u32 area = (message == WM_MOUSEMOVE) ? 1 : 2;
        //if (m_mouseTrackedArea != area)
        //{
        //    TRACKMOUSEEVENT tme_cancel = { sizeof(tme_cancel), (DWORD)TME_CANCEL, m_hWnd, 0 };
        //    TRACKMOUSEEVENT tme_track = { sizeof(tme_track), (DWORD)((message == WM_MOUSEMOVE) ? TME_LEAVE : TME_LEAVE | TME_NONCLIENT), m_hWnd, 0 };
        //    if (m_mouseTrackedArea != 0)
        //    {
        //        TrackMouseEvent(&tme_cancel);
        //    }
        //    TrackMouseEvent(&tme_track);
        //    m_mouseTrackedArea = area;
        //}

        POINT localPos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        if (message == WM_NCMOUSEMOVE) // WM_NCMOUSEMOVE are absolute coordinates.
        {
            ScreenToClient(m_hWnd, &localPos);
        }

        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = localPos.x;
        event->_clientCoordinates.m_y = localPos.y;
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseMoved;
        event->_modifers = 0;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
    {
        //const int area = (message == WM_MOUSELEAVE) ? 1 : 2;
        //if (m_mouseTrackedArea == area)
        //{
        //    m_mouseTrackedArea = 0;
        //}

        POINT absolutePos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        ClientToScreen(m_hWnd, &absolutePos);

        event::MouseInputEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::MouseInputEvent());
        event->_clientCoordinates.m_x = 0;
        event->_clientCoordinates.m_y = 0;
        event->_absoluteCoordinates.m_x = absolutePos.x;
        event->_absoluteCoordinates.m_y = absolutePos.y;
        event->_wheelValue = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
        event->_event = event::MouseInputEvent::MouseMoved;
        event->_modifers = 0;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);

        return FALSE;
    }

    }

    return DefWindowProc(m_hWnd, message, wParam, lParam);
}

LRESULT WindowWindows::HandleSystemEvents(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_receiver)
    {
        return DefWindowProc(m_hWnd, message, wParam, lParam);
    }

    switch (message)
    {
    case WM_CREATE:
    {
        event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
        event->_systemEvent = event::SystemEvent::Create;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_ACTIVATE:
    {
        BOOL active = LOWORD(wParam) != WA_INACTIVE;
        BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

        if (active && iconified)
        {
            active = FALSE;
        }
        m_params._isActive = active;
        return FALSE;
    }

    case WM_SETFOCUS:
    {
        m_params._isFocused = true;

        event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
        event->_systemEvent = event::SystemEvent::Focus;
        event->_flag = 0x1;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_KILLFOCUS:
    {
        m_params._isFocused = false;

        event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
        event->_systemEvent = event::SystemEvent::Focus;
        event->_flag = 0x0;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_SHOWWINDOW:
    {
        return FALSE;
    }

    case WM_CHAR:
    {
        event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
        event->_systemEvent = event::SystemEvent::TextInput;
        event->_flag = wParam;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    case WM_MOVE:
    {
        m_params._position.m_x = (s16)LOWORD(lParam);
        m_params._position.m_y = (s16)HIWORD(lParam);

        if (m_lastMoveEvent < m_currnetTime)
        {
            u32 id = this->ID();
            event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
            event->_systemEvent = event::SystemEvent::Move;
            event->_flag = lParam;
            event->_windowID = this->ID();

            m_receiver->sendEvent(event);
            m_lastMoveEvent = m_currnetTime;
        }
        return FALSE;
    }

    case WM_SIZE:
    {
        if (wParam != SIZE_MINIMIZED)
        {
            s32 width = (UINT)LOWORD(lParam);
            s32 height = (UINT)HIWORD(lParam);

            if (width > 0 && height > 0 && (m_params._size.m_width != width || m_params._size.m_height != height))
            {
                m_params._size.m_width = width;
                m_params._size.m_height = height;

                event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
                event->_systemEvent = event::SystemEvent::Resize;
                event->_flag = lParam;
                event->_windowID = this->ID();

                m_receiver->sendEvent(event);
            }
            m_lastSizeEvent = m_currnetTime;
        }
        return FALSE;
    }

    case WM_TIMER:
    {
        ++m_currnetTime;
        return FALSE;
    }

    case WM_DESTROY:
    {
        event::SystemEvent* event = V3D_PLACMENT_NEW(m_receiver->allocateInputEvent(), event::SystemEvent());
        event->_systemEvent = event::SystemEvent::Destroy;
        event->_windowID = this->ID();

        m_receiver->pushEvent(event);
        return FALSE;
    }

    default:
        return TRUE;
    }
}

LRESULT WindowWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);

        WindowWindows* window = reinterpret_cast<WindowWindows*>(cs->lpCreateParams);
        ASSERT(window, "Must be valid pointer");

        SetTimer(hWnd, window->m_timerID, 100, NULL); //milliseconds
        return window->HandleSystemEvents(message, wParam, lParam);
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
        ASSERT(window, "Must be valid pointer");

        switch (message)
        {
        case WM_ACTIVATE:
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        case WM_SHOWWINDOW:
        case WM_MOVE:
        case WM_SIZE:
        case WM_TIMER:
        case WM_CHAR:
            return window->HandleSystemEvents(message, wParam, lParam);

        case WM_DESTROY:
        {
            KillTimer(hWnd, window->m_timerID);
            return window->HandleSystemEvents(message, wParam, lParam);
        }

        default:
            return window->HandleInputMessage(message, wParam, lParam);
        }
    }
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
    m_keyCodes.add(event::KeyCode::KeyBackspace, 0x08);
    m_keyCodes.add(event::KeyCode::KeyTab, 0x09);
    m_keyCodes.add(event::KeyCode::KeyClear, 0x0C);
    m_keyCodes.add(event::KeyCode::KeyReturn, 0x0D);
    m_keyCodes.add(event::KeyCode::KeyShift, 0x10);
    m_keyCodes.add(event::KeyCode::KeyControl, 0x11);
    m_keyCodes.add(event::KeyCode::KeyAlt, 0x12);
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
    m_keyCodes.add(event::KeyCode::KeyLAlt, 0xA4);
    m_keyCodes.add(event::KeyCode::KeyRAlt, 0xA5);
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
