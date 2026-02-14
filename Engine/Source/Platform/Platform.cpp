#include "Platform.h"
#include "Window.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include <tchar.h>
#include <dwmapi.h>

int V3D_AssertYesNoDlg(const char* file, int line, const char* statement, const char* message, ...)
{
    const int size = 1024;
    char str[size];
    int pos = sprintf_s(str, "Assertion breakpoint: (%s)\n\nFile: %s\nLine: %d\n\n", statement, file, line);
    if (message)
    {
        va_list ap;
        va_start(ap, message);
        if (pos < size - 1)
            vsprintf_s(str + pos, size - pos, message, ap);
        va_end(ap);
    }

    strcat_s(str, message ? "\n\nDebug?" : "Debug?");

    return MessageBoxA(NULL, str, "Assert failed", MB_YESNO | MB_ICONERROR) == IDYES;
}

static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
{
    typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
    static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = nullptr;
    if (RtlVerifyVersionInfoFn == nullptr)
        if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
            RtlVerifyVersionInfoFn = (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
    if (RtlVerifyVersionInfoFn == nullptr)
        return FALSE;

    RTL_OSVERSIONINFOEXW versionInfo = { };
    ULONGLONG conditionMask = 0;
    versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    versionInfo.dwMajorVersion = major;
    versionInfo.dwMinorVersion = minor;
    VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0) ? TRUE : FALSE;
}

#define _IsWindowsVistaOrGreater()   _IsWindowsVersionOrGreater(HIBYTE(0x0600), LOBYTE(0x0600), 0) // _WIN32_WINNT_VISTA
#define _IsWindows8OrGreater()       _IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0) // _WIN32_WINNT_WIN8
#define _IsWindows8Point1OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0) // _WIN32_WINNT_WINBLUE
#define _IsWindows10OrGreater()      _IsWindowsVersionOrGreater(HIBYTE(0x0A00), LOBYTE(0x0A00), 0) // _WIN32_WINNT_WINTHRESHOLD / _WIN32_WINNT_WIN10

#ifndef DPI_ENUMS_DECLARED
typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    (DPI_AWARENESS_CONTEXT)-3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT)-4
#endif
typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);                     // Shcore.lib + dll, Windows 8.1+
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);        // Shcore.lib + dll, Windows 8.1+
typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)

#if defined(_MSC_VER) && !defined(NOGDI)
#pragma comment(lib, "gdi32")   // Link with gdi32.lib for GetDeviceCaps(). MinGW will require linking with '-lgdi32'
#endif

float GetDpiScaleForMonitor(void* monitor)
{
    UINT xdpi = 96, ydpi = 96;
    if (_IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
        if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr)
            GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
        if (GetDpiForMonitorFn != nullptr)
        {
            GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
            ASSERT(xdpi == ydpi, ""); // Please contact me if you hit this assert!
            return xdpi / 96.0f;
        }
    }
#ifndef NOGDI
    const HDC dc = ::GetDC(nullptr);
    xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
    ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
    ASSERT(xdpi == ydpi, ""); // Please contact me if you hit this assert!
    ::ReleaseDC(nullptr, dc);
#endif
    return xdpi / 96.0f;
}

float GetDpiScaleForHwnd(void* hwnd)
{
    HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
    return GetDpiScaleForMonitor(monitor);
}
#endif

namespace v3d
{
namespace platform
{

void Platform::setCursorIcon(CursorIcon icon)
{
#ifdef PLATFORM_WINDOWS
    // Show OS mouse cursor
    LPTSTR win32_cursor = IDC_ARROW;
    switch (icon)
    {
    case CursorIcon::Arrow:
        win32_cursor = IDC_ARROW;
        break;

    case CursorIcon::TextInput:
        win32_cursor = IDC_IBEAM;
        break;

    case CursorIcon::ResizeAll:
        win32_cursor = IDC_SIZEALL;
        break;

    case CursorIcon::ResizeEW:
        win32_cursor = IDC_SIZEWE;
        break;

    case CursorIcon::ResizeNS:
        win32_cursor = IDC_SIZENS;
        break;

    case CursorIcon::ResizeNESW:
        win32_cursor = IDC_SIZENESW;
        break;

    case CursorIcon::ResizeNWSE:
        win32_cursor = IDC_SIZENWSE;
        break;

    case CursorIcon::Hand:
        win32_cursor = IDC_HAND;
        break;

    case CursorIcon::Wait:
        win32_cursor = IDC_WAIT;
        break;

    case CursorIcon::Progress:
        win32_cursor = IDC_APPSTARTING;
        break;

    case CursorIcon::NotAllowed:
        win32_cursor = IDC_NO;
        break;

    default:
        win32_cursor = IDC_ARROW;
    }
    ::SetCursor(::LoadCursor(NULL, win32_cursor));
#else
    ASSERT("not impl");
#endif //PLATFORM_WINDOWS
}

void Platform::hideCursor()
{
    ::ShowCursor(FALSE);
}

void Platform::showCursor()
{
    ::ShowCursor(TRUE);
}

math::Point2D Platform::getCursorPosition()
{
#ifdef PLATFORM_WINDOWS
    POINT mouse;
    GetCursorPos(&mouse);

    return math::Point2D(static_cast<s32>(mouse.x), static_cast<s32>(mouse.y));
#else
    ASSERT("not impl");
#endif //PLATFORM_WINDOWS

    return math::Point2D(0U, 0U);
}

void Platform::setCursorPostion(const math::Point2D& point)
{
#ifdef PLATFORM_WINDOWS
    SetCursorPos(point._x, point._y);
#else
    ASSERT("not impl");
#endif //PLATFORM_WINDOWS
}

#ifdef PLATFORM_WINDOWS
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD   dwType;         // Must be 0x1000.
    LPCSTR  szName;         // Pointer to name (in user addr space).
    DWORD   dwThreadID;     // Thread ID (-1=caller thread).
    DWORD   dwFlags;        // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

bool Platform::setThreadName(std::thread& thread, const std::string& name)
{
    DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread.native_handle()));

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name.c_str();
    info.dwThreadID = threadId;
    info.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try 
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        return false;
    }
#pragma warning(pop)
    return false;
}

bool Platform::setThreadAffinityMask(std::thread& thread, u64 mask)
{
    HANDLE hThread = static_cast<HANDLE>(thread.native_handle());
    if (::SetThreadAffinityMask(hThread, (DWORD_PTR)mask))
    {
        return true;
    }

    return false;
}

bool Platform::setThreadPriority(std::thread& thread, s32 priority)
{
    HANDLE hThread = static_cast<HANDLE>(thread.native_handle());
    static s32 priorityTable[5]
    {
        THREAD_PRIORITY_LOWEST,
        THREAD_PRIORITY_BELOW_NORMAL,
        THREAD_PRIORITY_NORMAL,
        THREAD_PRIORITY_ABOVE_NORMAL,
        THREAD_PRIORITY_HIGHEST
    };

    ASSERT(priority >= THREAD_PRIORITY_LOWEST && priority <= THREAD_PRIORITY_HIGHEST, "range out");
    if (::SetThreadPriority(hThread, priorityTable[priority]));
    {
        return true;
    }

    return false;
}

#else
bool Platform::setThreadName(std::thread& thread, const std::string& name)
{
    return false;
}

bool Platform::setThreadAffinityMask(std::thread& thread, u64 mask)
{
    return false;
}

bool Platform::setThreadPriority(std::thread& thread, s32 priority)
{
    return false;
}
#endif //PLATFORM_WINDOWS

#ifdef PLATFORM_WINDOWS
std::wstring Platform::utf8ToWide(const c8* in)
{
    s32 size = MultiByteToWideChar(CP_UTF8, 0, in, -1, nullptr, 0);
    // size includes terminating null; std::wstring adds null automatically
    std::wstring out(static_cast<size_t>(size) - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, in, -1, &out[0], size);
    return out;
}

std::string Platform::wideToUtf8(const w16* in)
{
    s32 size = WideCharToMultiByte(CP_UTF8, 0, in, -1, nullptr, 0, nullptr, nullptr);
    // size includes terminating null; std::string adds null automatically
    std::string out(static_cast<size_t>(size) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, in, -1, &out[0], size, nullptr, nullptr);
    return out;
}

void Platform::enumDisplayMonitors(const DisplayMonitorsFunc& func)
{
    auto updateMonitors_EnumFunc = [](HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM param) -> BOOL
    {
        MONITORINFO info = {};
        info.cbSize = sizeof(MONITORINFO);
        if (!::GetMonitorInfo(monitor, &info))
        {
            return TRUE;
        }

        auto& func = *reinterpret_cast<const DisplayMonitorsFunc*>(param);

        math::Rect rcMonitor(info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom);
        math::Rect rcWork(info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom);
        f32 dpi = GetDpiScaleForMonitor(monitor);
        return func(rcMonitor, rcWork, dpi, info.dwFlags & MONITORINFOF_PRIMARY, monitor);
    };

    ::EnumDisplayMonitors(nullptr, nullptr, updateMonitors_EnumFunc, reinterpret_cast<LPARAM>(&func));
}
f32 Platform::getDpiScaleForWindow(const Window* window)
{
    return GetDpiScaleForHwnd(window->getWindowHandle());
}
#endif //PLATFORM_WINDOWS

} //namespace platform
} //namespace v3d
