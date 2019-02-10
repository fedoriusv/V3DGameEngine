cmake_minimum_required(VERSION 3.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##############################################################
#Examples
option(BUILD_EXAMPLES "Generate Examples" ON)
##############################################################
#Render SDK
option(BUILD_VULKAN_SDK "Build use Vulkan SDK" ON)
##############################################################
#ThirdParty
option(CRC32_LIB "Use Crc32c lib" ON)
#https://github.com/google/crc32c.git
option(ASSIMP_LIB "Use Assimp lib" ON)
#https://github.com/assimp/assimp.git
option(SPIRV_INSTALL_FROM_GITHUB "implicity install SpirV" OFF)
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
set(SOLUTION_NAME V3DGE)
set(ENGINE_NAME V3DEngine)

project(${SOLUTION_NAME})
##############################################################

#Pathes
##############################################################
set(SOURCE_DIR Engine/Source)
set(ENGINE_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(LIB_INCLUDE_DIRECTORY 
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR} 
	#${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/Renderer
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/ThirdParty)

#Crc32c	
if (CRC32_LIB)
	set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/crc32c/include)
endif()
#Assimp
if (ASSIMP_LIB)
	set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/assimp/include)
endif()

include_directories(BEFORE ${LIB_INCLUDE_DIRECTORY})
##############################################################
 
#Configuration
##############################################################
set(INCLUDE_DIRECTORY_PATHES "")

#Vulkan
if(BUILD_VULKAN_SDK)
    set(VULKAN_SDK_DEFINE "-DVULKAN_RENDER")
    #VK_SDK_PATH - must be added to system variables
    set(VULKAN_SDK_PATH $ENV{VK_SDK_PATH})
	if ("${VULKAN_SDK_PATH}" STREQUAL "")
		message(FATAL_ERROR "Vulkan SDK not found. Please add VK_SDK_PATH to system variable")
	endif()
    set(VULKAN_SDK_INCLUDE_DIRECTORY "-I${VULKAN_SDK_PATH}/Include")
    set(VULKAN_SDK_LIB_DIRECTORY ${VULKAN_SDK_PATH}/Lib)
    message("-- Vulkan SDK Path " ${VULKAN_SDK_PATH})
	
	#SpirV
	set (SPIRV_FOLDER ${ENGINE_PROJECT_DIR}/Engine/Libs/spirv)
	if ((IS_DIRECTORY ${SPIRV_FOLDER}/shaderc AND IS_DIRECTORY ${SPIRV_FOLDER}/SPIRV-Cross) OR SPIRV_INSTALL_FROM_GITHUB)
		set(VULKAN_SDK_INCLUDE_DIRECTORY "${VULKAN_SDK_INCLUDE_DIRECTORY} -I${SPIRV_FOLDER}")
		set(VULKAN_SDK_DEFINE "${VULKAN_SDK_DEFINE} -DUSE_SPIRV")
		if (WIN32)
			set(SHADERC_COMBINED_LIB_FILE_DEBUG ${SPIRV_FOLDER}/shaderc/libshaderc/Debug/shaderc_combined.lib)
			set(SHADERC_COMBINED_LIB_FILE_RELEASE ${SPIRV_FOLDER}/shaderc/libshaderc/Release/shaderc_combined.lib)
			
			set(SPIRV_CROSS_CPP_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-cpp.lib)
			set(SPIRV_CROSS_CORE_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-core.lib)
			set(SPIRV_CROSS_GLSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-glsl.lib)
			#set(SPIRV_CROSS_MSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-msl.lib)  
			set(SPIRV_CROSS_CPP_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-cpp.lib)
			set(SPIRV_CROSS_CORE_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-core.lib)
			set(SPIRV_CROSS_GLSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-glsl.lib)
			#set(SPIRV_CROSS_MSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-msl.lib)
		endif()
	endif()
endif()


#Compiler Flags
if(WIN32)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP")
#   Debug
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DUSE_LOGGER -D_CRT_SECURE_NO_WARNINGS ${VULKAN_SDK_DEFINE}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${VULKAN_SDK_INCLUDE_DIRECTORY}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MTd /std:c++17")
    set(CMAKE_DEBUG_POSTFIX "_d")
#   Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DUSE_LOGGER ${VULKAN_SDK_DEFINE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${VULKAN_SDK_INCLUDE_DIRECTORY}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /std:c++17")
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
file(GLOB RENDER_OBJECT_HEADERS ${SOURCE_DIR}/Renderer/Object/*.h)
file(GLOB RENDER_OBJECT_SOURCES ${SOURCE_DIR}/Renderer/Object/*.cpp)
file(GLOB RESOURCE_HEADERS ${SOURCE_DIR}/Resource/*.h)
file(GLOB RESOURCE_SOURCES ${SOURCE_DIR}/Resource/*.cpp)
file(GLOB STREAM_HEADERS ${SOURCE_DIR}/Stream/*.h)
file(GLOB STREAM_SOURCES ${SOURCE_DIR}/Stream/*.cpp)
file(GLOB SCENE_HEADERS ${SOURCE_DIR}/Scene/*.h)
file(GLOB SCENE_SOURCES ${SOURCE_DIR}/Scene/*.cpp)

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
source_group("Renderer\\Object" FILES ${RENDER_OBJECT_HEADERS} ${RENDER_OBJECT_SOURCES})
source_group("Resource" FILES ${RESOURCE_HEADERS} ${RESOURCE_SOURCES})
source_group("Stream" FILES ${STREAM_HEADERS} ${STREAM_SOURCES})
source_group("Scene" FILES ${SCENE_HEADERS} ${SCENE_SOURCES})

if (BUILD_VULKAN_SDK)
	source_group("Renderer\\Vulkan" FILES ${VULKAN_HEADERS} ${VULKAN_SOURCES})
endif()

add_library(${ENGINE_NAME} STATIC 
    ${MAIN_HEADERS} ${MAIN_SOURCES}
    ${CORE_HEADERS} ${CORE_SOURCES}
    ${PLATFORM_HEADERS} ${PLATFORM_SOURCES}
    ${UTILS_HEADERS} ${UTILS_SOURCES}
    ${EVENT_HEADERS} ${EVENT_SOURCES}
    ${RENDERER_HEADERS} ${RENDERER_SOURCES}
	${RENDER_OBJECT_HEADERS} ${RENDER_OBJECT_SOURCES}
	${RESOURCE_HEADERS} ${RESOURCE_SOURCES}
	${STREAM_HEADERS} ${STREAM_SOURCES}
    ${SCENE_HEADERS} ${SCENE_SOURCES}
    ${VULKAN_HEADERS} ${VULKAN_SOURCES})
	
set(BUILD_DEPENDENCIES "")

#Link Libs
##############################################################
#Crc32c	
if (CRC32_LIB)
	target_link_libraries(${ENGINE_NAME} crc32c)
	set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} crc32c)
endif()
#Assimp	
if (ASSIMP_LIB)
	target_link_libraries(${ENGINE_NAME} assimp)
	set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} assimp)
endif()
#Spirv
if (BUILD_VULKAN_SDK)
	if ((IS_DIRECTORY ${SPIRV_FOLDER}/shaderc AND IS_DIRECTORY ${SPIRV_FOLDER}/SPIRV-Cross) OR SPIRV_INSTALL_FROM_GITHUB)
		target_link_libraries(${ENGINE_NAME} optimized ${SHADERC_COMBINED_LIB_FILE_RELEASE} debug ${SHADERC_COMBINED_LIB_FILE_DEBUG})
		target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_CPP_RELEASE}           debug ${SPIRV_CROSS_CPP_DEBUG})
		target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_CORE_RELEASE}          debug ${SPIRV_CROSS_CORE_DEBUG})
		target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_GLSL_RELEASE}          debug ${SPIRV_CROSS_GLSL_DEBUG})
		#target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_MSL_RELEASE}           debug ${SPIRV_CROSS_MSL_DEBUG})
	endif()
endif()

link_directories(${ENGINE_NAME} ${VULKAN_SDK_LIB_DIRECTORY})
    
##############################################################

#Libs
##############################################################
if (CRC32_LIB)
	message("-- Add Crc32 lib")
	add_subdirectory(Engine/Libs/crc32c)
    set_target_properties(crc32c PROPERTIES FOLDER Libs)
	message("-- Crc32 lib added")
endif()
if (ASSIMP_LIB)
	message("-- Add Assimp lib")
	add_subdirectory(Engine/Libs/assimp)
    set_target_properties(assimp PROPERTIES FOLDER Libs)
	message("-- Crc32 lib added")
endif()
##############################################################

#Examples
##############################################################
if (BUILD_EXAMPLES)
    message("-- Create Examples:")
    add_subdirectory(Examples/1.CreateWindow)
    add_subdirectory(Examples/2.ClearColor)
	add_subdirectory(Examples/3.SimpleDraw)
	add_subdirectory(Examples/4.RenderTarget)
    set_target_properties(1.CreateWindow 2.ClearColor 3.SimpleDraw 4.RenderTarget PROPERTIES FOLDER Examples)
    message("-- ----------------")
endif ()
##############################################################

add_dependencies(${ENGINE_NAME} ${BUILD_DEPENDENCIES})

##############################################################

#install external libs
##############################################################

if (WIN32 AND SPIRV_INSTALL_FROM_GITHUB)
message("-- Installing SPIRV:")
execute_process(COMMAND ${SPIRV_FOLDER}/install.bat WORKING_DIRECTORY ${SPIRV_FOLDER})
message("-- SPIRV Installed")
endif()
##############################################################