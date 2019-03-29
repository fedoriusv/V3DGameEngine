// V3D Engine (c) 2018 Fred Nekrasov
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
#include <atomic>

#include <fstream>
#include <iostream>

#include <numeric>
#include <variant>

//#include <time.h>
#include <ctime>
#include <chrono>

#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>


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
