#pragma once

#include "Configuration.h"

#include <assert.h>

#include <string>
#include <array>
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>
#include <functional>
#include <utility>
#include <type_traits>

#include <thread>
#include <memory>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <condition_variable>

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <numeric>
#include <random>

#include <ctime>
#include <chrono>

#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <variant>

#include "Math/Core.h"
#include "Utils/CRC32.h"
#include "Utils/Spinlock.h"
#include "Memory/Memory.h"
#include "Renderer/Color.h"

extern int V3D_AssertYesNoDlg(const char* file, int line, const char* statement, const char* message, ...);
inline void V3D_Assert(const char* file, int line, const char* statement, const char* message, ...)
{
    std::cerr << "Assert: " << statement << ", message: " << message << ", file " << file << ", line " << line << '\n';
#if defined(DEVELOPMENT)
    if (V3D_AssertYesNoDlg(file, line, statement, message, 0))
    {
        assert(false);
    }
#else
    assert(false);
#endif
}

#ifdef DEBUG
#   define ASSERT(cond, ...)                                                        \
            {                                                                       \
                if (!(cond)) V3D_Assert(__FILE__, __LINE__, #cond, ##__VA_ARGS__);  \
            }
#else //DEBUG
//#   define RELEASE_WITH_DEBUG
#   ifdef RELEASE_WITH_DEBUG
inline void crashFunc(bool x)
{
    if (x) return;

    int* volatile ptr = 0;
    *ptr = 1;
}
#       define ASSERT(x, message) crashFunc(x)
#   else
#       define ASSERT(x, message)
#   endif //RELEASE_WITH_DEBUG
#endif //

#define NOT_IMPL ASSERT(false, "not impl")

#ifdef __GNUC__
#   define BREAK() __builtin_trap()
#elif defined(_MSC_VER)
#   define BREAK() __debugbreak()
#else //
#   define BREAK() __builtin_debugtrap()
#endif //

#if defined(PLATFORM_WINDOWS)
#   define NOMINMAX
#   include <windows.h>
#   undef CreateSemaphore
#   undef CreateEvent
typedef HINSTANCE NativeInstance;
typedef HWND      NativeWindow;

#elif defined(PLATFORM_XBOX)
#   define NOMINMAX
#   define NODRAWTEXT
#   define NOGDI
#   define NOBITMAP
#   define NOMCX
#   define NOSERVICE
#   define NOHELP
#   include <windows.h>
typedef HINSTANCE NativeInstance;
typedef HWND      NativeWindow;

#elif defined (PLATFORM_ANDROID)
#   include "Platform/Android/AndroidCommon.h"
typedef ANativeWindow* NativeWindow;
typedef ANativeActivity* NativeInstance;

#else //PLATFORM
typedef void*     NativeInstance;
typedef void*     NativeWindow;
#endif //PLATFORM
