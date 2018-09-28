// V3D Engine (c) 2018 Fred Nekrasov
#pragma once

#include "Configuration.h"

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <queue>
#include <array>
#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <fstream>
#include <numeric>
#include <functional>
#include <utility>
#include <type_traits>

#include <time.h>
#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>

#include "Core/Core.h"

#ifdef _DEBUG
#   include <assert.h>
#   define ASSERT(x, message) assert(x && message)
#else //_DEBUG
#   define ASSERT(x, message)
#endif //_DEBUG

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

#ifdef PLATFORM_WINDOWS
#   include <windows.h>
typedef HINSTANCE NativeInstance;
typedef HWND      NativeWindows;
#   undef max
#   undef min
#   undef CreateSemaphore
#   undef CreateEvent
#else //
typedef void*     NativeInstance;
typedef void*     NativeWindows;
#endif //
