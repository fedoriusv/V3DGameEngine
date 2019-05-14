// V3D Engine (c) 2018-2019 Fred Nekrasov
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

//PLatform
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#   define PLATFORM_WINDOWS
#   if defined(_MSC_VER) && (_MSC_VER < 1910)
#       error "Only Microsoft Visual Studio 2017 RTW (15.0) and later are supported."
#   endif
#endif

#if defined(__APPLE__) || defined(MACOSX)
#   define PLATFORM_MACOSX
#   error Unsupported platform
#endif

#if defined(LINUX) || defined(_LINUX)
#   define PLATFORM_LINUX
#   error Unsupported platform
#endif

#if defined(_ANDROID)
#   define PLATFORM_ANDROID
#   if defined(__ANDROID_API__) && (__ANDROID_API__ < 26)
#       error "Support Android NDK R18b and later"
#   endif
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////

//Render
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef VULKAN_RENDER
#   define VULKAN_VERSION_MAJOR 1
#   define VULKAN_VERSION_MINOR 1

#   define VULKAN_DEBUG 0
#   define VULKAN_LAYERS_CALLBACKS 1
#   if VULKAN_LAYERS_CALLBACKS
#       define VULKAN_VALIDATION_LAYERS_CALLBACK 1
#       define VULKAN_RENDERDOC_LAYER 0
#       define VULKAN_DEBUG_MARKERS 1
#   endif //VULKAN_LAYERS_CALLBACKS

#   define VULKAN_STATISTICS 0
#   define VULKAN_DUMP 1
#   if VULKAN_DUMP
#   define VULKAN_DUMP_FILE "VulkanCommandsDump.log"
#   endif //VULKAN_DUMP
#endif //VULKAN_RENDER


#ifdef D3D_RENDER
#   define D3D_VERSION_MAJOR 12
#   define D3D_VERSION_MINOR 0
#   define D3D_DEBUG 1
#endif //D3D_RENDER

#define DEBUG_COMMAND_LIST 0

/////////////////////////////////////////////////////////////////////////////////////////////////////
