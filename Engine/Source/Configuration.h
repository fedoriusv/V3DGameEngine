// V3D Engine 2024 Fred Nekrasov
#pragma once

//Version 2.0.0
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENGINE_VERSION_MAJOR       2
#define ENGINE_VERSION_MINOR       0
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

//Render
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef D3D_RENDER
#   define D3D_VERSION_MAJOR 12
#   define D3D_VERSION_MINOR 0

#   if defined(NTDDI_VERSION) && (NTDDI_VERSION < 0x0A00000A/*NTDDI_WIN10_FE*/)
#       error "Windows SDK must be 10.0.20348 or later. Make sure you have a proper version"
#   endif

#   define D3D_DEBUG 0 //Detailed logs
#   define D3D_DEBUG_LAYERS 1
#   if D3D_DEBUG_LAYERS
#       define D3D_DEBUG_LIVE_REPORT 1
#       define D3D_DEBUG_LAYERS_CALLBACK 1
#   endif
#   define D3D_DEBUG_MARKERS 1
#   define D3D_DEBUG_PIX_MARKERS 0
#   define D3D_DRED_ENABLE 0
#   define D3D_AFTERMATH_ENABLE 0

#   define D3D_MAKE_VERSION(major, minor) (((major) << 22) | ((minor) << 12))

#   define D3D_VERSION_12_0 D3D_MAKE_VERSION(12, 0)
#   define D3D_VERSION_12_1 D3D_MAKE_VERSION(12, 1)
#   define D3D_VERSION_12_2 D3D_MAKE_VERSION(12, 2)

#   define D3D_CURRENT_VERSION D3D_MAKE_VERSION(D3D_VERSION_MAJOR, D3D_VERSION_MINOR)

#endif //D3D_RENDER

#define DEBUG_OBJECT_MEMORY 1
#define FRAME_PROFILER_ENABLE 0

/////////////////////////////////////////////////////////////////////////////////////////////////////

/*
* @brief Internal memory managment.
*/
#define MEMORY_MANAGMENT 1
#define MEMORY_DEBUG 1

#define DEBUG_COMMAND_LIST 0 //Logging command list

/////////////////////////////////////////////////////////////////////////////////////////////////////