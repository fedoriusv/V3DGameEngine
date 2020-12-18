// V3D Engine 2018-2021 Fred Nekrasov
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

#if defined(_WIN32) && _WIN32
#   error Unsupported platform. Used x64 only.
#endif

#if defined(_WIN64) && _WIN64
#   define PLATFORM_WINDOWS
#   if defined(_MSC_VER) && (_MSC_VER < 1920)
#       error "Only Microsoft Visual Studio 2019 RTW (16.0) and later are supported."
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


/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOGGER
#   ifndef PLATFORM_ANDROID
#       define HIGHLIGHTING_LOGS 1
#   endif
#endif //USED_LOGGER

//Render
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef VULKAN_RENDER
#   ifdef PLATFORM_ANDROID
#       define VULKAN_VERSION_MAJOR 1
#       define VULKAN_VERSION_MINOR 1
#   else
#       define VULKAN_VERSION_MAJOR 1
#       define VULKAN_VERSION_MINOR 2
#   endif

#   define VULKAN_DEBUG 1
#   define VULKAN_LAYERS_CALLBACKS 1
#   if VULKAN_LAYERS_CALLBACKS
#       define VULKAN_VALIDATION_LAYERS_CALLBACK 1
#       define VULKAN_RENDERDOC_LAYER 0
#       define VULKAN_DEBUG_MARKERS 1
#   endif //VULKAN_LAYERS_CALLBACKS

#   define VULKAN_STATISTICS 0
#   define VULKAN_DUMP 0
#   if VULKAN_DUMP
#   define VULKAN_DUMP_FILE "VulkanCommandsDump.log"
#   endif //VULKAN_DUMP

#define VULKAN_VERSION_1_0 (VULKAN_VERSION_MAJOR == 1 && VULKAN_VERSION_MINOR == 0)
#define VULKAN_VERSION_1_1 (VULKAN_VERSION_MAJOR == 1 && VULKAN_VERSION_MINOR == 1)
#define VULKAN_VERSION_1_2 (VULKAN_VERSION_MAJOR == 1 && VULKAN_VERSION_MINOR == 2)

#endif //VULKAN_RENDER


#ifdef D3D_RENDER
#   define D3D_VERSION_MAJOR 12
#   define D3D_VERSION_MINOR 0

#   define D3D_DEBUG 1
#   define D3D_DEBUG_LAYERS 1
#endif //D3D_RENDER

#define DEBUG_COMMAND_LIST 0
#define DEBUG_OBJECT_MEMORY 1
#define FRAME_PROFILER_ENABLE 1

/////////////////////////////////////////////////////////////////////////////////////////////////////
