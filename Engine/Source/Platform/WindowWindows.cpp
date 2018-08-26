#include "WindowWindows.h"
#include "Utils/Logger.h"

#include <winuser.h>

namespace v3d
{
namespace platform
{

WindowWindows::WindowWindows(const WindowParam& params)
    : Window(params)
    , m_hInstance(NULL)
    , m_hWnd(NULL)
{
    LOG_DEBUG("WindowWindows::WindowWindows: Created Windows window %llx", this);
}

bool WindowWindows::initialize()
{
    HINSTANCE m_hInstance = GetModuleHandle(NULL);

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
    m_params._size.width += borderRect.right - borderRect.left;
    m_params._size.height += borderRect.bottom - borderRect.top;

    // create window
    ASSERT(!m_hWnd, "Already exist");
    m_hWnd = CreateWindowEx(dwExStyle, m_classname.c_str(), __TEXT("Test"), dwStyle, m_params._position.x, m_params._position.y, m_params._size.width, m_params._size.height, NULL, NULL, m_hInstance, this);
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

LRESULT WindowWindows::HandleEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        //
        return TRUE;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
       //
        return TRUE;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
    {
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
            return window->HandleEvents(hWnd, message, wParam, lParam);
        }
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} //namespace platform
} //namespace v3d
