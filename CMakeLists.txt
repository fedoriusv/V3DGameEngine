cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##############################################################
#Examples
option(BUILD_EXAMPLES "Generate Examples" ON)
##############################################################
#Render SDK
option(BUILD_VULKAN_SDK "Build use Vulkan SDK" OFF)
##############################################################
#ThirdParty
#Crc32c lib is required"
#https://github.com/google/crc32c.git
option(ASSIMP_LIB "Use Assimp lib" OFF)
#https://github.com/assimp/assimp.git
option(STB_LIB "Use Stb lib" OFF)
#https://github.com/nothings/stb.git
option(GIL_LIB "Use gli lib" OFF)
#https://github.com/g-truc/gli.git
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

if(TARGET_WIN AND WIN32)
    message("-- Platform Windows")
	include(Config/Windows64.cmake)
    add_definitions(-D_WINAPI -DUNICODE -D_UNICODE)
elseif(TARGET_ANDROID)
    message("-- Platform Android")
	include(Config/Android.cmake)
    add_definitions(-D_ANDROID -DUNICODE -D_UNICODE)
else()
    message(FATAL_ERROR "Unknown platform. Only Platform Windows | Android supported")
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
	${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/ThirdParty)

set(EXTRA_LIB_DEFINE "")
#Crc32c	
set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/crc32c/include)

#Assimp
if (ASSIMP_LIB)
	set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/assimp/include)
	set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_ASSIMP")
endif()
#Stb
if (STB_LIB)
	set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_STB")
endif()
#Gli
if (GIL_LIB)
	set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_GLI")
endif()
 
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
    set(VULKAN_SDK_INCLUDE_DIRECTORY "${VULKAN_SDK_PATH}/Include")
    set(VULKAN_SDK_LIB_DIRECTORY ${VULKAN_SDK_PATH}/Lib)
    message("-- Vulkan SDK Path " ${VULKAN_SDK_PATH})

    #SpirV
    set(SPIRV_FOLDER ${ENGINE_PROJECT_DIR}/Engine/Libs/spirv)
    if ((IS_DIRECTORY ${SPIRV_FOLDER}/shaderc AND IS_DIRECTORY ${SPIRV_FOLDER}/SPIRV-Cross) OR SPIRV_INSTALL_FROM_GITHUB)
        set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK_INCLUDE_DIRECTORY} ${SPIRV_FOLDER})
        set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK_INCLUDE_DIRECTORY} ${SPIRV_FOLDER}/shaderc/libshaderc/include)
        set(VULKAN_SDK_DEFINE "${VULKAN_SDK_DEFINE} -DUSE_SPIRV")
        if (TARGET_WIN)
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
		elseif (TARGET_ANDROID)
			set(SHADERC_COMBINED_LIB_FILE_DEBUG ${SPIRV_FOLDER}/shaderc/libshaderc/Debug/shaderc_combined.a)
            set(SHADERC_COMBINED_LIB_FILE_RELEASE ${SPIRV_FOLDER}/shaderc/libshaderc/Release/shaderc_combined.a)

            set(SPIRV_CROSS_CPP_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-cpp.a)
            set(SPIRV_CROSS_CORE_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-core.a)
            set(SPIRV_CROSS_GLSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-glsl.a)
            #set(SPIRV_CROSS_MSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-msl.a)  
            set(SPIRV_CROSS_CPP_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-cpp.a)
            set(SPIRV_CROSS_CORE_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-core.a)
            set(SPIRV_CROSS_GLSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-glsl.a)
            #set(SPIRV_CROSS_MSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-msl.a)
        endif()
    endif()
endif()

#Compiler Flags
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DUSE_LOGGER -D_CRT_SECURE_NO_WARNINGS ${VULKAN_SDK_DEFINE} ${EXTRA_LIB_DEFINE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${EXTRA_LIB_DEFINE}")
##############################################################

#Source
##############################################################
file(GLOB MAIN_HEADERS ${SOURCE_DIR}/*.h)
file(GLOB MAIN_SOURCES ${SOURCE_DIR}/*.cpp)
file(GLOB CORE_HEADERS ${SOURCE_DIR}/Core/*.h  ${SOURCE_DIR}/Core/*.inl)
file(GLOB CORE_SOURCES ${SOURCE_DIR}/Core/*.cpp)
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

if (TARGET_WIN)
	file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Platform.h ${SOURCE_DIR}/Platform/Window.h ${SOURCE_DIR}/Platform/WindowWindows.h)
	file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Platform.cpp ${SOURCE_DIR}/Platform/Window.cpp ${SOURCE_DIR}/Platform/WindowWindows.cpp)
elseif (TARGET_ANDROID)
	file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Platform.h ${SOURCE_DIR}/Platform/Window.h ${SOURCE_DIR}/Platform/WindowAndroid.h)
	file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Platform.cpp ${SOURCE_DIR}/Platform/Window.cpp ${SOURCE_DIR}/Platform/WindowAndroid.cpp)
endif()

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
	
target_include_directories(${ENGINE_NAME} BEFORE 
	PRIVATE ${LIB_INCLUDE_DIRECTORY}
	PRIVATE $<$<BOOL:BUILD_VULKAN_SDK>:${VULKAN_SDK_INCLUDE_DIRECTORY}>)
    
set(BUILD_DEPENDENCIES "")

#Link Libs
##############################################################
#Crc32c	
target_link_libraries(${ENGINE_NAME} crc32c)
set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} crc32c)

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
message("-- Add Crc32 lib")
add_subdirectory(Engine/Libs/crc32c)
set_target_properties(crc32c PROPERTIES FOLDER Libs/crc32c)
set_target_properties(crc32c_arm64 PROPERTIES FOLDER Libs/crc32c)
set_target_properties(crc32c_sse42 PROPERTIES FOLDER Libs/crc32c)
message("-- Crc32 lib added")

if (ASSIMP_LIB)
    message("-- Add Assimp lib")
    add_subdirectory(Engine/Libs/assimp)
    set_target_properties(assimp PROPERTIES FOLDER Libs/assimp)
    target_compile_options(assimp PRIVATE -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    set_target_properties(IrrXML PROPERTIES FOLDER Libs/assimp)
    set_target_properties(uninstall PROPERTIES FOLDER Libs/assimp)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER Libs/assimp)
    set_target_properties(zlib PROPERTIES FOLDER Libs/assimp)  
    set_target_properties(zlibstatic PROPERTIES FOLDER Libs/assimp)
if(TARGET_WIN)
    target_compile_options(zlib PRIVATE $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
    target_compile_options(zlibstatic PRIVATE $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
endif()

    message("-- Assimp lib added")
endif()

##############################################################

#Examples
##############################################################
if (BUILD_EXAMPLES)
    set(BUILD_PROJECTS 
        1.CreateWindow
        2.ClearColor
        3.DrawMesh
        4.DrawScene
        5.Test
    )
    #set(BUILD_PROJECTS #[[1.CreateWindow 2.ClearColor]] 3.DrawMesh 4.DrawScene 5.Test)
    message("-- Create Examples:")
    
    foreach (PROJ ${BUILD_PROJECTS})
        add_subdirectory(Examples/${PROJ})
    endforeach(PROJ)
    
    set_target_properties(${BUILD_PROJECTS} PROPERTIES FOLDER Examples)
    set_target_properties(${BUILD_PROJECTS} PROPERTIES INCLUDE_DIRECTORIES ${ENGINE_PROJECT_DIR}/${SOURCE_DIR})
    message("-- ----------------")
endif ()
##############################################################

if (BUILD_DEPENDENCIES)
    add_dependencies(${ENGINE_NAME} ${BUILD_DEPENDENCIES})
endif()
##############################################################

#install external libs
##############################################################

if (WIN32 AND SPIRV_INSTALL_FROM_GITHUB)
    message("-- Installing SPIRV:")
    execute_process(COMMAND ${SPIRV_FOLDER}/install.bat WORKING_DIRECTORY ${SPIRV_FOLDER})
    message("-- SPIRV Installed")
endif()
##############################################################