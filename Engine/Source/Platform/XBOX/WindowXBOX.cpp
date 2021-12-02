#include "WindowXBOX.h"
#include "Event/InputEventKeyboard.h"
#include "Event/InputEventMouse.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_XBOX
#include <XGameRuntimeInit.h>
#include <XSystem.h>
#include <XGameErr.h>

namespace v3d
{
namespace platform
{

WindowXBOX::WindowXBOX(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
    , m_hInstance(NULL)
    , m_hWnd(NULL)
{
    LOG_DEBUG("WindowXBOX::WindowXBOX: Created Windows window %llx", this);
}

bool WindowXBOX::initialize()
{
    //HRESULT result = XGameRuntimeInitialize();
    //if (FAILED(result))
    //{
    //    if (result == E_GAMERUNTIME_DLL_NOT_FOUND || result == E_GAMERUNTIME_VERSION_MISMATCH)
    //    {
    //        LOG_ERROR("WindowXBOX::initialize : Game Runtime is not installed on this system or needs updating");
    //    }

    //    ASSERT(false, "XGameRuntimeInitialize is failed");
    //    return false;
    //}

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

//#ifdef _GAMING_XBOX
//    g_plmSuspendComplete = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
//    g_plmSignalResume = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
//    if (!g_plmSuspendComplete || !g_plmSignalResume)
//        return 1;
//
//    if (RegisterAppStateChangeNotification([](BOOLEAN quiesced, PVOID context)
//        {
//            if (quiesced)
//            {
//                ResetEvent(g_plmSuspendComplete);
//                ResetEvent(g_plmSignalResume);
//
//                // To ensure we use the main UI thread to process the notification, we self-post a message
//                PostMessage(reinterpret_cast<HWND>(context), WM_USER, 0, 0);
//
//                // To defer suspend, you must wait to exit this callback
//                (void)WaitForSingleObject(g_plmSuspendComplete, INFINITE);
//            }
//            else
//            {
//                SetEvent(g_plmSignalResume);
//            }
//        }, hwnd, &hPLM))
//        return 1;
//#endif

    //XSystemDeviceType type = XSystemGetDeviceType();
    //XSystemAnalyticsInfo info = XSystemGetAnalyticsInfo();

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
    MSG msg = {};
    PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
    DestroyWindow(m_hWnd);
    m_hWnd = NULL;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    UnregisterClass(m_classname.c_str(), hInstance);
    hInstance = NULL;

    XGameRuntimeUninitialize();
}

WindowXBOX::~WindowXBOX()
{
    LOG_DEBUG("WindowWindows::~WindowWindows");
    ASSERT(!m_hWnd, "Not destroyed");
}

void WindowXBOX::minimize()
{
    NOT_IMPL;
 }

void WindowXBOX::maximize()
{
    NOT_IMPL;
}

void WindowXBOX::restore()
{
    NOT_IMPL;
}

void WindowXBOX::setFullScreen(bool value)
{
}

void WindowXBOX::setResizeble(bool value)
{
}

void WindowXBOX::setTextCaption(const std::string& text)
{
}

void WindowXBOX::setPosition(const core::Point2D& pos)
{
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
    WindowXBOX* window = reinterpret_cast<WindowXBOX*>(lpCreateParams);

    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_ACTIVATEAPP:
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

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
