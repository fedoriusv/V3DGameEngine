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
    m_hWnd = CreateWindowEx(dwExStyle, m_classname.c_str(), __TEXT("Test"), dwStyle, m_params._position.x, m_params._position.y, m_params._size.width, m_params._size.height, NULL, NULL, m_hInstance, NULL);
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
}

void WindowWindows::setResizeble(bool value)
{
    //TODO:
}

void WindowWindows::setTextCaption(const std::string& text)
{
    SetWindowTextA(m_hWnd, text.c_str());
}

void WindowWindows::setPosition(const core::Point2D& pos)
{
    //TODO:
}

bool WindowWindows::isMaximized() const
{
    return false;
}

bool WindowWindows::isMinimized() const
{
    return false;
}

bool WindowWindows::isActive() const
{
    return false;
}

bool WindowWindows::isFocused() const
{
    return false;
}

LRESULT WindowWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hWnd, 0, (LONG_PTR)cs->lpCreateParams);

        return TRUE;
    }

    case WM_ACTIVATE:
    {
        BOOL focused = LOWORD(wParam) != WA_INACTIVE;
        BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

        if (focused && iconified)
        {
            focused = FALSE;
        }

        return TRUE;
    }

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

    case WM_TIMER:
    {
        //        {
        //            KillTimer(hWnd, 0);
        //        }
        return TRUE;
    }

    case WM_SIZE:
    {
        return TRUE;
    }

    case WM_SHOWWINDOW:
    {
        return TRUE;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return TRUE;
    }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} //namespace platform
} //namespace v3d
