#pragma once

#include "Configuration.h"

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
#include "Memory/Memory.h"
#include "Renderer/Color.h"

#ifdef DEBUG
#   include <assert.h>
#   define ASSERT(x, message) assert(x && message)
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
#endif //DEBUG

#define PLATFORM_BREAK() __debugbreak()

#define NOT_IMPL ASSERT(false, "not impl")

#ifdef __GNUC__
#   define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#   if defined(DEPRECATED)
#       undef DEPRECATED
#   endif //DEPRECATED
#   define DEPRECATED __declspec(deprecated)
#else //
#   pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#   define DEPRECATED
#endif //


#if defined(PLATFORM_WINDOWS)
#   define NOMINMAX
#   include <windows.h>
#   undef CreateSemaphore
#   undef CreateEvent
typedef HINSTANCE NativeInstance;
typedef HWND      NativeWindows;

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
typedef HWND      NativeWindows;

#elif defined (PLATFORM_ANDROID)
#   include "Platform/Android/AndroidCommon.h"
typedef ANativeWindow*   NativeWindows;
typedef ANativeActivity* NativeInstance;

#else //PLATFORM
typedef void*     NativeInstance;
typedef void*     NativeWindows;
#endif //PLATFORM
