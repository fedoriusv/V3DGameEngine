// V3D Engine 2025 Fred Nekrasov
#pragma once

//Version 2.1.0
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENGINE_VERSION_MAJOR       2
#define ENGINE_VERSION_MINOR       1
#define ENGINE_VERSION_REVISION    0

//Config
/////////////////////////////////////////////////////////////////////////////////////////////////////

//PLatform
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(_WIN32) && _WIN32) && (!defined(_WIN64) && _WIN64 == 0)
#   error Unsupported platform. Used x64 only.
#endif

#if defined(_WIN64) && _WIN64
#   define PLATFORM_WINDOWS
#   include "sdkddkver.h"

#   if defined(_MSC_VER) && (_MSC_VER < 1930)
#       error "Only Microsoft Visual Studio 2022 (17.0.1) or later is supported."
#   endif

#   pragma warning(disable: 26812)
#endif

#if defined(__APPLE__) || defined(MACOSX)
#   define PLATFORM_MACOSX
#   error Unsupported platform
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#   define PLATFORM_ANDROID
#   if defined(__ANDROID_API__) && (__ANDROID_API__ < 26)
#       error "Support Android NDK R18b and later"
#   endif
#endif

#if (defined(__LINUX__) || defined(LINUX))
#   define PLATFORM_LINUX
#   error Unsupported platform
#endif

#if defined(_GAMING_XBOX) && defined(_GAMING_XBOX_SCARLETT)
#   define PLATFORM_XBOX
#   define PLATFORM_XBOX_SCARLETT
#   undef PLATFORM_WINDOWS
#endif

#if defined(_GAMING_XBOX) && defined(_GAMING_XBOX_XBOXONE)
#   error Unsupported platform
#endif


#if defined (DYNAMIC_LIB)
#   ifdef V3D_BUILD_DLL
#       define V3D_API __declspec(dllexport)
#   else
#       define V3D_API __declspec(dllimport)
#   endif // 
#else
#       define V3D_API
#endif //DYNAMIC_LIB

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOGGER
#   if defined(PLATFORM_WINDOWS)
#       define HIGHLIGHTING_LOGS 1
#   endif
#endif //USED_LOGGER

//Debug
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_OBJECT_MEMORY 1
#define FRAME_PROFILER_ENABLE (1 && !DEBUG)

/////////////////////////////////////////////////////////////////////////////////////////////////////

/*
* @brief Internal memory managment.
*/
#define MEMORY_MANAGMENT 1
#define MEMORY_DEBUG 1

#define DEBUG_COMMAND_LIST 0 //Logging command list

/////////////////////////////////////////////////////////////////////////////////////////////////////