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

#include <assert.h>
#include <time.h>
#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "Core/Core.h"

#ifdef _DEBUG
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