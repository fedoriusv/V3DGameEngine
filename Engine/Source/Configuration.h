// V3D Engine (c) 2018 Fred Nekrasov
#pragma once

//Version 2.0.0
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENGINE_VERSION_MAJOR       2
#define ENGINE_VERSION_MINOR       0
#define ENGINE_VERSION_REVISION    0

//Config
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOGGER
#   define HIGHLIGHTING_LOGS 1
#endif //USED_LOGGER

//Compiler
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#   error "Only Microsoft Visual Studio 14.0 and later are supported."
#endif

//PLatform
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#   define PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) || defined(MACOSX)
#   define PLATFORM_MACOSX
#   error Unsupported platform
#endif

#if defined(LINUX) || defined(_LINUX)
#   define PLATFORM_LINUX
#   error Unsupported platform
#endif

#if defined(__ANDROID__) && defined(__ARM_ARCH)
#   define PLATFORM_ANDROID
#   error Unsupported platform
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////

//Render
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef VULKAN_RENDER
#   define VULKAN_VERSION_MAJOR 1
#   define VULKAN_VERSION_MINOR 1
#   define VULKAN_VERSION_PATCH 0

#   define VULKAN_DEBUG 1
#   define VULKAN_VALIDATION_LAYERS_CALLBACK 1
#   define VULKAN_RENDERDOC_LAYER 0
#   define VULKAN_DEBUG_MARKERS 0

#   ifdef PLATFORM_WINDOWS
#       define VK_USE_PLATFORM_WIN32_KHR
#   elif PLATFORM_LINUX
#       define VK_USE_PLATFORM_XLIB_KHR
#   elif PLATFORM_ANDROID
#       define VK_USE_PLATFORM_ANDROID_KHR
#   endif
#endif //VULKAN_RENDER
/////////////////////////////////////////////////////////////////////////////////////////////////////
