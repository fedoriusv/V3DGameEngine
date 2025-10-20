#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VULKAN_RENDER

//Vulkan version
#ifdef PLATFORM_ANDROID
#    define VULKAN_VERSION_MAJOR 1
#    define VULKAN_VERSION_MINOR 1
#else
#    define VULKAN_VERSION_MAJOR 1
#    define VULKAN_VERSION_MINOR 4
#endif

//Debug
#define VULKAN_DEBUG 0 //Log every render command (optional)


//Validation
#if defined(DEBUG) && !defined(DEVELOPMENT)
#   define VULKAN_LAYERS_CALLBACKS 1
#else
#   define VULKAN_LAYERS_CALLBACKS 0
#endif

#if VULKAN_LAYERS_CALLBACKS
#    define VULKAN_VALIDATION_LAYERS_CALLBACK 1
#    define VULKAN_RENDERDOC_LAYER 0
#    define VULKAN_DEBUG_MARKERS 1
#    define VULKAN_DEBUG_SHADER 0
#endif //VULKAN_LAYERS_CALLBACKS


#define VULKAN_STATISTICS 0

#define VULKAN_DUMP 0
#if VULKAN_DUMP
#    define VULKAN_DUMP_FILE "VulkanCommandsDump.log"
#endif //VULKAN_DUMP

#define VULKAN_MAKE_VERSION(major, minor) (((major) << 22) | ((minor) << 12))

#define VULKAN_VERSION_1_0 VULKAN_MAKE_VERSION(1, 0)
#define VULKAN_VERSION_1_1 VULKAN_MAKE_VERSION(1, 1)
#define VULKAN_VERSION_1_2 VULKAN_MAKE_VERSION(1, 2)
#define VULKAN_VERSION_1_3 VULKAN_MAKE_VERSION(1, 3)
#define VULKAN_VERSION_1_4 VULKAN_MAKE_VERSION(1, 4)

#define VULKAN_CURRENT_VERSION VULKAN_MAKE_VERSION(VULKAN_VERSION_MAJOR, VULKAN_VERSION_MINOR)

// Vulkan features

// see https://www.khronos.org/blog/streamlining-render-passes
#define DYNAMIC_RENDERING 0

#endif //VULKAN_RENDER

/////////////////////////////////////////////////////////////////////////////////////////////////////
