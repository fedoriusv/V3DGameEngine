cmake_minimum_required(VERSION 3.0.0)

##############################################################
#Examples
option(BUILD_EXAMPLES "Generate Examples" ON)
##############################################################
#Render SDK
option(BUILD_VULKAN_SDK "Build use Vulkan SDK" ON)
##############################################################

if(COMPILER_MSVC)
    message("-- Configuring MSVC Compiler")
elseif(COMPILER_GCC)
    message("-- Configuring GCC Compiler")
elseif(COMPILER_CLANG)
    message("-- Configuring Clang Compiler")
else()
    message(FATAL_ERROR "Unknown compiler. Only COMPILER_MSVC|COMPILER_GCC|COMPILER_CLANG supported")
endif()

if(WIN32)
    message("-- Platform Windows")
    add_definitions(-D_WINAPI -D_CONSOLE -DUNICODE -D_UNICODE)
elseif(UNIX AND NOT APPLE)
    message("-- Platform Linux")
    add_definitions(-D_LINUX -DUNICODE -D_UNICODE)
elseif(APPLE)
    message("-- Platform MacOSX")
    add_definitions(-DMACOSX -DUNICODE -D_UNICODE)
else()
    message(FATAL_ERROR "Unknown platform. Only Platrorm Windows|Platrorm Linux|Platrorm MacOSX supported")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only support x64 Architecture")
endif()
##############################################################

#Project
##############################################################
set(SOLUTION_NAME V3DGameEngine)
set(ENGINE_NAME V3DEngine)

project(${SOLUTION_NAME})
##############################################################

#Pathes
##############################################################
set(SOURCE_DIR Engine/Source)
set(ENGINE_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(LIB_INCLUDE_DIRECTORY 
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR} 
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/Renderer 
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/ThirdParty)

include_directories(BEFORE ${LIB_INCLUDE_DIRECTORY})
##############################################################
 
#Configuration
##############################################################
#Vulkan
if(BUILD_VULKAN_SDK)
    set(VULKAN_SDK_DEFINE "-DVULKAN_RENDER")
    #VK_SDK_PATH - must be added to system variables
    set(VULKAN_SDK_PATH $ENV{VK_SDK_PATH})
	if ("${VULKAN_SDK_PATH}" STREQUAL "")
		message(FATAL_ERROR "Vulkan SDK not found. Please add VK_SDK_PATH system variable")
	endif()
    set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK_PATH}/Include)
    set(VULKAN_SDK_LIB_DIRECTORY ${VULKAN_SDK_PATH}/Lib)
    message("-- Vulkan SDK Path " ${VULKAN_SDK_PATH})
endif()

#Compiler Flags
if(WIN32)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP")
#   Debug
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DUSE_LOGGER ${VULKAN_SDK_DEFINE}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -I${VULKAN_SDK_INCLUDE_DIRECTORY}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MTd -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_DEBUG_POSTFIX "_d")
#   Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DUSE_LOGGER ${VULKAN_SDK_DEFINE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -I${VULKAN_SDK_INCLUDE_DIRECTORY}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()
##############################################################

#Source
##############################################################
file(GLOB MAIN_HEADERS ${SOURCE_DIR}/*.h)
file(GLOB MAIN_SOURCES ${SOURCE_DIR}/*.cpp)
file(GLOB CORE_HEADERS ${SOURCE_DIR}/Core/*.h  ${SOURCE_DIR}/Core/*.inl)
file(GLOB CORE_SOURCES ${SOURCE_DIR}/Core/*.cpp)
file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/*.h)
file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/*.cpp)
file(GLOB UTILS_HEADERS ${SOURCE_DIR}/Utils/*.h)
file(GLOB UTILS_SOURCES ${SOURCE_DIR}/Utils/*.cpp)
file(GLOB EVENT_HEADERS ${SOURCE_DIR}/Event/*.h)
file(GLOB EVENT_SOURCES ${SOURCE_DIR}/Event/*.cpp)
file(GLOB RENDERER_HEADERS ${SOURCE_DIR}/Renderer/*.h)
file(GLOB RENDERER_SOURCES ${SOURCE_DIR}/Renderer/*.cpp)
file(GLOB OBJECT_HEADERS ${SOURCE_DIR}/Object/*.h)
file(GLOB OBJECT_SOURCES ${SOURCE_DIR}/Object/*.cpp)

if (BUILD_VULKAN_SDK)
file(GLOB VULKAN_HEADERS ${SOURCE_DIR}/Renderer/Vulkan/*.h)
file(GLOB VULKAN_SOURCES ${SOURCE_DIR}/Renderer/Vulkan/*.cpp)
endif()
##############################################################

#Filters
##############################################################
source_group("" FILES ${MAIN_HEADERS} ${MAIN_SOURCES})
source_group("Core" FILES ${CORE_HEADERS} ${CORE_SOURCES})
source_group("Platform" FILES ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
source_group("Utils" FILES ${UTILS_HEADERS} ${UTILS_SOURCES})
source_group("Event" FILES ${EVENT_HEADERS} ${EVENT_SOURCES})
source_group("Renderer" FILES ${RENDERER_HEADERS} ${RENDERER_SOURCES})
source_group("Object" FILES ${OBJECT_HEADERS} ${OBJECT_SOURCES})

source_group("Renderer\\Vulkan" FILES ${VULKAN_HEADERS} ${VULKAN_SOURCES})

add_library(${ENGINE_NAME} STATIC 
    ${MAIN_HEADERS} ${MAIN_SOURCES}
    ${CORE_HEADERS} ${CORE_SOURCES}
    ${PLATFORM_HEADERS} ${PLATFORM_SOURCES}
    ${UTILS_HEADERS} ${UTILS_SOURCES}
    ${EVENT_HEADERS} ${EVENT_SOURCES}
    ${RENDERER_HEADERS} ${RENDERER_SOURCES}
	${OBJECT_HEADERS} ${OBJECT_SOURCES}
    ${VULKAN_HEADERS} ${VULKAN_SOURCES})

link_directories(${ENGINE_NAME} ${VULKAN_SDK_LIB_DIRECTORY})
    
##############################################################

#Examples
##############################################################
if (BUILD_EXAMPLES)
    message("-- Create Examples:")
    add_subdirectory(Examples/1.CreateWindow)
    add_subdirectory(Examples/2.ClearColor)
    message("-- ----------------")
endif ()
##############################################################