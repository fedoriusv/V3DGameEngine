#include "Platform.h"

namespace v3d
{
namespace platform
{

math::Point2D Platform::getCursorPosition()
{
#ifdef PLATFORM_WINDOWS
    POINT mouse;
    GetCursorPos(&mouse);

    return math::Point2D(static_cast<s32>(mouse.x), static_cast<s32>(mouse.y));
#endif //PLATFORM_WINDOWS

    return math::Point2D(0U, 0U);
}

void Platform::setCursorPostion(const math::Point2D & point)
{
#ifdef PLATFORM_WINDOWS
    SetCursorPos(point.m_x, point.m_y);
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
#else
bool Platform::setThreadName(std::thread& thread, const std::string& name)
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
#endif

} //namespace platform
} //namespace v3d
