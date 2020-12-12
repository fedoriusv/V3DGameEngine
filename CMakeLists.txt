cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##############################################################
#Examples
option(BUILD_EXAMPLES "Generate Examples" ON)
##############################################################
#Render SDK
option(BUILD_VULKAN_SDK "Build use Vulkan SDK" ON)
option(BUILD_DIRECTX_SDK "Use DirectX SDK" ON)
##############################################################
#ThirdParty
include(Config/ThirdParty.cmake)
##############################################################

#Project
##############################################################
set(SOLUTION_NAME V3DGE)
set(ENGINE_NAME V3DEngine)

project(${SOLUTION_NAME})
##############################################################

if(COMPILER_MSVC)
    message(STATUS "Configuring MSVC Compiler")
elseif(COMPILER_GCC)
    message(STATUS "Configuring GCC Compiler")
elseif(COMPILER_CLANG)
    message(STATUS "Configuring Clang Compiler")
else()
    message(FATAL_ERROR "Unknown compiler. Only COMPILER_MSVC|COMPILER_GCC|COMPILER_CLANG supported")
endif()

if(TARGET_WIN AND WIN32)
    message(STATUS "Platform Windows")
    include(Config/Windows64.cmake)
    add_definitions(-D_WINAPI -DUNICODE -D_UNICODE)
elseif(TARGET_ANDROID)
    message(STATUS "Platform Android")
    include(Config/Android.cmake)
    set(BUILD_DIRECTX_SDK OFF)
    add_definitions(-D_ANDROID -DUNICODE -D_UNICODE)
else()
    message(FATAL_ERROR "Unknown platform. Only Platform Windows | Android supported")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only support x64 Architecture")
endif()
##############################################################

#Pathes
##############################################################
set(SOURCE_DIR Engine/Source)
set(ENGINE_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(LIB_INCLUDE_DIRECTORY 
    ${ENGINE_PROJECT_DIR}/${SOURCE_DIR}
    ${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/Platform
    ${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/ThirdParty)

foreach(INCLUDE_SUFFIX ${EXTRA_LIB_INCLUDE_SUFFIX})
    set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/${SOURCE_DIR}/ThirdParty/${INCLUDE_SUFFIX})
endforeach()

#Crc32c	
set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/crc32c/include)

#Assimp
if(ASSIMP_LIB)
    set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Libs/assimp/include)
    set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_ASSIMP")
endif()

##############################################################

#Vulkan
if(BUILD_VULKAN_SDK)
    set(VULKAN_SDK_DEFINE "-DVULKAN_RENDER")
    if (TARGET_ANDROID)
         set(VULKAN_SDK_DEFINE "${VULKAN_SDK_DEFINE} -DVULKAN_DYNAMIC")
    endif()
    #VK_SDK_PATH - must be added to system variables
    set(VULKAN_SDK_PATH $ENV{VK_SDK_PATH})
    if("${VULKAN_SDK_PATH}" STREQUAL "")
        message(FATAL_ERROR "Vulkan SDK not found. Please add VK_SDK_PATH to system variable")
    endif()
    set(VULKAN_SDK_INCLUDE_DIRECTORY "${VULKAN_SDK_PATH}/Include")
    set(VULKAN_SDK_LIB_DIRECTORY ${VULKAN_SDK_PATH}/Lib)
    message(STATUS "Vulkan SDK Path " ${VULKAN_SDK_PATH})

    #SpirV
    set(SPIRV_FOLDER ${ENGINE_PROJECT_DIR}/Engine/Libs/spirv)
    if((IS_DIRECTORY ${SPIRV_FOLDER}/shaderc AND IS_DIRECTORY ${SPIRV_FOLDER}/SPIRV-Cross) OR SPIRV_INSTALL_FROM_GITHUB)
        set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK_INCLUDE_DIRECTORY} ${SPIRV_FOLDER})
        set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK_INCLUDE_DIRECTORY} ${SPIRV_FOLDER}/shaderc/libshaderc/include)
        set(VULKAN_SDK_DEFINE "${VULKAN_SDK_DEFINE} -DUSE_SPIRV")
        if(TARGET_WIN)
            set(SHADERC_COMBINED_LIB_FILE_DEBUG ${SPIRV_FOLDER}/shaderc/build_windows/libshaderc/Debug/shaderc_combined.lib)
            set(SHADERC_COMBINED_LIB_FILE_RELEASE ${SPIRV_FOLDER}/shaderc/build_windows/libshaderc/Release/shaderc_combined.lib)
            set(SPIRV_CROSS_CPP_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Debug/spirv-cross-cppd.lib)
            set(SPIRV_CROSS_CORE_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Debug/spirv-cross-cored.lib)
            set(SPIRV_CROSS_GLSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Debug/spirv-cross-glsld.lib)
            #set(SPIRV_CROSS_MSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-msl.lib)  
            set(SPIRV_CROSS_CPP_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Release/spirv-cross-cpp.lib)
            set(SPIRV_CROSS_CORE_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Release/spirv-cross-core.lib)
            set(SPIRV_CROSS_GLSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_windows/Release/spirv-cross-glsl.lib)
            #set(SPIRV_CROSS_MSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-msl.lib)
        elseif(TARGET_ANDROID)
           #set(SHADERC_COMBINED_LIB_FILE_DEBUG ${SPIRV_FOLDER}/shaderc/build_android/Debug/libshaderc/shaderc_combined.a)
           set(SHADERC_COMBINED_LIB_FILE_RELEASE ${SPIRV_FOLDER}/shaderc/build_android/Release/libshaderc/libshaderc_combined.a)
           #set(SPIRV_CROSS_CPP_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-cpp.a)
           #set(SPIRV_CROSS_CORE_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-core.a)
           #set(SPIRV_CROSS_GLSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-glsl.a)
           #set(SPIRV_CROSS_MSL_DEBUG ${SPIRV_FOLDER}/SPIRV-Cross/Debug/spirv-cross-msl.a)  
           set(SPIRV_CROSS_CPP_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_android/Release/libspirv-cross-cpp.a)
           set(SPIRV_CROSS_CORE_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_android/Release/libspirv-cross-core.a)
           set(SPIRV_CROSS_GLSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/build_android/Release/libspirv-cross-glsl.a)
            #set(SPIRV_CROSS_MSL_RELEASE ${SPIRV_FOLDER}/SPIRV-Cross/Release/spirv-cross-msl.a)
        endif()
    endif()
endif()

#DirectX
if (BUILD_DIRECTX_SDK)
    set(DIRECTX_SDK_DEFINE "-DD3D_RENDER")
    message(STATUS "DirectX SDK Path " ${DirectX_PREFIX_PATH})
endif()

#Compiler Flags
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${EXTRA_LIB_DEFINE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${EXTRA_LIB_DEFINE}")
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

file(GLOB COMMON_PLATFORM_HEADERS ${SOURCE_DIR}/Platform/*.h)
file(GLOB COMMON_PLATFORM_SOURCES ${SOURCE_DIR}/Platform/*.cpp)
if (TARGET_WIN)
    file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Windows/*.h)
    file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Windows/*.cpp)
elseif (TARGET_ANDROID)
    file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Android/*.h)
    file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Android/*.cpp ${SOURCE_DIR}/Platform/Android/*.c)
endif()

if (BUILD_VULKAN_SDK)
    file(GLOB VULKAN_HEADERS ${SOURCE_DIR}/Renderer/Vulkan/*.h)
    file(GLOB VULKAN_SOURCES ${SOURCE_DIR}/Renderer/Vulkan/*.cpp)
endif()

if (BUILD_DIRECTX_SDK)
    file(GLOB DIRECTX_HEADERS ${SOURCE_DIR}/Renderer/D3D12/*.h)
    file(GLOB DIRECTX_SOURCES ${SOURCE_DIR}/Renderer/D3D12/*.cpp)
endif()
##############################################################

#Filters
##############################################################
source_group("" FILES ${MAIN_HEADERS} ${MAIN_SOURCES})
source_group("Core" FILES ${CORE_HEADERS} ${CORE_SOURCES})
source_group("Platform" FILES ${COMMON_PLATFORM_HEADERS} ${COMMON_PLATFORM_SOURCES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
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

if (BUILD_DIRECTX_SDK)
    source_group("Renderer\\D3D12" FILES ${DIRECTX_HEADERS} ${DIRECTX_SOURCES})
endif()

add_library(${ENGINE_NAME} STATIC 
    ${MAIN_HEADERS} ${MAIN_SOURCES}
    ${CORE_HEADERS} ${CORE_SOURCES}
    ${COMMON_PLATFORM_HEADERS} ${COMMON_PLATFORM_SOURCES}
    ${PLATFORM_HEADERS} ${PLATFORM_SOURCES}
    ${UTILS_HEADERS} ${UTILS_SOURCES}
    ${EVENT_HEADERS} ${EVENT_SOURCES}
    ${RENDERER_HEADERS} ${RENDERER_SOURCES}
    ${RENDER_OBJECT_HEADERS} ${RENDER_OBJECT_SOURCES}
    ${RESOURCE_HEADERS} ${RESOURCE_SOURCES}
    ${STREAM_HEADERS} ${STREAM_SOURCES}
    ${SCENE_HEADERS} ${SCENE_SOURCES}
    $<$<BOOL:BUILD_VULKAN_SDK>:${VULKAN_HEADERS} ${VULKAN_SOURCES}>
    $<$<BOOL:BUILD_DIRECTX_SDK>:${DIRECTX_HEADERS} ${DIRECTX_SOURCES}>
    )

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
        if (TARGET_ANDROID)
            target_link_libraries(${ENGINE_NAME} ${SHADERC_COMBINED_LIB_FILE_RELEASE})
            target_link_libraries(${ENGINE_NAME} ${SPIRV_CROSS_CPP_RELEASE})
            target_link_libraries(${ENGINE_NAME} ${SPIRV_CROSS_CORE_RELEASE})
            target_link_libraries(${ENGINE_NAME} ${SPIRV_CROSS_GLSL_RELEASE})
        else()
            target_link_libraries(${ENGINE_NAME} optimized ${SHADERC_COMBINED_LIB_FILE_RELEASE} debug ${SHADERC_COMBINED_LIB_FILE_DEBUG})
            target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_CPP_RELEASE}           debug ${SPIRV_CROSS_CPP_DEBUG})
            target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_CORE_RELEASE}          debug ${SPIRV_CROSS_CORE_DEBUG})
            target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_GLSL_RELEASE}          debug ${SPIRV_CROSS_GLSL_DEBUG})
            #target_link_libraries(${ENGINE_NAME} optimized ${SPIRV_CROSS_MSL_RELEASE}           debug ${SPIRV_CROSS_MSL_DEBUG})
        endif()
    endif()
endif()

link_directories(${ENGINE_NAME} ${VULKAN_SDK_LIB_DIRECTORY})
    
##############################################################

#Libs
##############################################################
message(STATUS "Add Crc32 lib")
add_subdirectory(Engine/Libs/crc32c)
set_target_properties(crc32c PROPERTIES FOLDER Libs/crc32c)
set_target_properties(crc32c_arm64 PROPERTIES FOLDER Libs/crc32c)
set_target_properties(crc32c_sse42 PROPERTIES FOLDER Libs/crc32c)
message(STATUS "Crc32 lib added")

if (ASSIMP_LIB)
    message(STATUS "Add Assimp lib")
    add_subdirectory(Engine/Libs/assimp)
    target_compile_options(assimp PRIVATE -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -frtti)
if(TARGET_WIN)
    set_target_properties(assimp PROPERTIES FOLDER Libs/assimp)
    set_target_properties(IrrXML PROPERTIES FOLDER Libs/assimp)
    set_target_properties(uninstall PROPERTIES FOLDER Libs/assimp)
    set_target_properties(zlib PROPERTIES FOLDER Libs/assimp)  
    set_target_properties(zlibstatic PROPERTIES FOLDER Libs/assimp)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER Libs/assimp)
    
    target_compile_options(zlib PRIVATE $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
    target_compile_options(zlibstatic PRIVATE $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
endif()

    message(STATUS "Assimp lib added")
endif()

##############################################################

#Examples
##############################################################
if (BUILD_EXAMPLES)
#   set(BUILD_PROJECTS #[[1.CreateWindow 2.ClearColor]] 3.DrawMesh 4.DrawScene 5.Test)
    file(READ "ExamplesList.txt" STRING_BUILD_EXAMPLES)
    string(REPLACE "\n" ";" BUILD_EXAMPLES ${STRING_BUILD_EXAMPLES})

    message(STATUS "----------------")
    message(STATUS "Create Examples:")
    foreach (PROJ ${BUILD_EXAMPLES})
        add_subdirectory(Examples/${PROJ})
    endforeach(PROJ)
    
    set_target_properties(${BUILD_EXAMPLES} PROPERTIES FOLDER Examples)
    set_target_properties(${BUILD_EXAMPLES} PROPERTIES INCLUDE_DIRECTORIES ${ENGINE_PROJECT_DIR}/${SOURCE_DIR})
    message(STATUS "----------------")
endif ()
##############################################################

if (BUILD_DEPENDENCIES)
    add_dependencies(${ENGINE_NAME} ${BUILD_DEPENDENCIES})
endif()
##############################################################

#install external libs
##############################################################

if (SPIRV_INSTALL_FROM_GITHUB AND WIN32)
    message(STATUS "Installing SPIRV:")
    if (TARGET_WIN)
        execute_process(COMMAND ${SPIRV_FOLDER}/install_windows_platform.bat WORKING_DIRECTORY ${SPIRV_FOLDER})
    elseif (TARGET_ANDROID)
        execute_process(COMMAND ${SPIRV_FOLDER}/install_android_platform.bat WORKING_DIRECTORY ${SPIRV_FOLDER})
    endif()
    message(STATUS "SPIRV Installed")
endif()
##############################################################