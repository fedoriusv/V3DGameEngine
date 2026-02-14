cmake_minimum_required(VERSION 3.15)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##############################################################
#Examples
##############################################################
option(BUILD_EXAMPLES "Generate Examples" ON)

##############################################################
#Render SDK
##############################################################
option(BUILD_VULKAN_SDK_OPTION "Use Vulkan SDK" ON)
option(BUILD_DIRECTX_SDK_OPTION "Use DirectX SDK" OFF)

##############################################################
#ThirdParty
##############################################################

#Crc32c lib is required"
#https://github.com/google/crc32c.git

#Json lib is required"
#https://github.com/nlohmann/json.git

#DirectXMath lib is required"
#https://github.com/microsoft/DirectXMath.git

option(STB_LIB "Use STB" ON)
#https://github.com/nothings/stb.git

option(GLI_LIB "Use GLI. Uses GLM" ON)
#https://github.com/g-truc/gli.git
#https://github.com/g-truc/glm.git

option(ASSIMP_LIB "Use Assimp lib" ON)
#https://github.com/assimp/assimp.git

option(HWCPipe_LIB_OPTION "Use HWCPipe lib(Only for Android Target)" OFF)
#https://github.com/ARM-software/HWCPipe.git

option(SPIRV_INSTALL_FROM_GITHUB "Implicity install SpirV" OFF)
#https://github.com/google/shaderc.git
#https://github.com/KhronosGroup/SPIRV-Cross.git

option (DXCompiler_LIB_OPTION "Use custom DirectXShaderCompiler lib" OFF)
#https://github.com/microsoft/DirectXShaderCompiler.git

option(PROFILER_TRACY_LIB "Enable tracy profiler for CPU/GPU" ON)
#https://github.com/wolfpld/tracy.git

option(IMGUI_SOURCE "Enable imgui source files" ON)
#https://github.com/ocornut/imgui.git

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

set(PROJECT_PLATFORM "")
if(TARGET_WIN AND WIN32)
    message(STATUS "Platform Windows")
    include(Config/Windows64.cmake)
    set(BUILD_VULKAN_SDK ${BUILD_VULKAN_SDK_OPTION})
    set(BUILD_DIRECTX_SDK ${BUILD_DIRECTX_SDK_OPTION})
    set(DXCompiler_LIB ${DXCompiler_LIB_OPTION})
elseif(TARGET_ANDROID)
    message(STATUS "Platform Android")
    include(Config/Android.cmake)
    set(BUILD_VULKAN_SDK ${BUILD_VULKAN_SDK_OPTION})
    set(HWCPipe_LIB ${HWCPipe_LIB_OPTION})
    set(DXCompiler_LIB ON)
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
    ${ENGINE_PROJECT_DIR}/Engine/External
    ${ENGINE_PROJECT_DIR}/Engine/ThirdParty)

foreach(INCLUDE_SUFFIX ${THIRD_PARTY_LIB_INCLUDE_SUFFIX})
    set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/ThirdParty/${INCLUDE_SUFFIX})
endforeach()

foreach(INCLUDE_SUFFIX ${THIRD_PARTY_SOURCE_INCLUDE_SUFFIX})
    set(LIB_INCLUDE_DIRECTORY ${LIB_INCLUDE_DIRECTORY} ${ENGINE_PROJECT_DIR}/Engine/Source/ThirdParty/${INCLUDE_SUFFIX})
endforeach()

##############################################################
#Vulkan
##############################################################
if(BUILD_VULKAN_SDK)
    set(VULKAN_SDK_DEFINE "-DVULKAN_RENDER")
    if(TARGET_ANDROID)
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
    set(SPIRV_FOLDER ${ENGINE_PROJECT_DIR}/Engine/External/spirv)
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

##############################################################
#DirectX
##############################################################
if(BUILD_DIRECTX_SDK)
    set(DIRECTX_SDK_DEFINE "-DD3D_RENDER")
    message(STATUS "DirectX SDK Path " ${DirectX_PREFIX_PATH})
endif()

##############################################################
#Compiler Flags
##############################################################
set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Profile;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${THIRD_PARTY_LIB_DEFINE}")
set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEVELOPMENT} -DDEBUG -DDEVELOPMENT -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${THIRD_PARTY_LIB_DEFINE}")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -DPROFILE -DUSE_LOGGER ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${THIRD_PARTY_LIB_DEFINE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${VULKAN_SDK_DEFINE} ${DIRECTX_SDK_DEFINE} ${THIRD_PARTY_LIB_DEFINE}")

##############################################################
#Source
##############################################################
file(GLOB MAIN_HEADERS ${SOURCE_DIR}/*.h)
file(GLOB MAIN_SOURCES ${SOURCE_DIR}/*.cpp)
file(GLOB MATH_HEADERS ${SOURCE_DIR}/Math/*.h ${SOURCE_DIR}/Math/*.inl)
file(GLOB MATH_SOURCES ${SOURCE_DIR}/Math/*.cpp)
file(GLOB UTILS_HEADERS ${SOURCE_DIR}/Utils/*.h)
file(GLOB UTILS_SOURCES ${SOURCE_DIR}/Utils/*.cpp)
file(GLOB THIRDPARTY_HEADERS ${SOURCE_DIR}/ThirdParty/*.h ${SOURCE_DIR}/ThirdParty/*.hpp)
file(GLOB THIRDPARTY_SOURCES ${SOURCE_DIR}/ThirdParty/*.cpp)
file(GLOB EVENTS_HEADERS ${SOURCE_DIR}/Events/*.h)
file(GLOB EVENTS_SOURCES ${SOURCE_DIR}/Events/*.cpp)
file(GLOB INPUT_EVENTS_HEADERS ${SOURCE_DIR}/Events/Input/*.h)
file(GLOB INPUT_EVENTS_SOURCES ${SOURCE_DIR}/Events/Input/*.cpp)
file(GLOB GAME_EVENTS_HEADERS ${SOURCE_DIR}/Events/Game/*.h)
file(GLOB GAME_EVENTS_SOURCES ${SOURCE_DIR}/Events/Game/*.cpp)
file(GLOB RENDERER_HEADERS ${SOURCE_DIR}/Renderer/*.h)
file(GLOB RENDERER_SOURCES ${SOURCE_DIR}/Renderer/*.cpp)
file(GLOB RENDER_TECHNIQUES_HEADERS ${SOURCE_DIR}/RenderTechniques/*.h)
file(GLOB RENDER_TECHNIQUES_SOURCES ${SOURCE_DIR}/RenderTechniques/*.cpp)
file(GLOB RESOURCE_HEADERS ${SOURCE_DIR}/Resource/*.h)
file(GLOB RESOURCE_SOURCES ${SOURCE_DIR}/Resource/*.cpp)
file(GLOB RESOURCE_DECODER_HEADERS ${SOURCE_DIR}/Resource/Decoder/*.h)
file(GLOB RESOURCE_DECODER_SOURCES ${SOURCE_DIR}/Resource/Decoder/*.cpp)
file(GLOB RESOURCE_LOADER_HEADERS ${SOURCE_DIR}/Resource/Loader/*.h)
file(GLOB RESOURCE_LOADER_SOURCES ${SOURCE_DIR}/Resource/Loader/*.cpp)
file(GLOB MEMORY_HEADERS ${SOURCE_DIR}/Memory/*.h)
file(GLOB MEMORY_SOURCES ${SOURCE_DIR}/Memory/*.cpp)
file(GLOB STREAM_HEADERS ${SOURCE_DIR}/Stream/*.h)
file(GLOB STREAM_SOURCES ${SOURCE_DIR}/Stream/*.cpp)
file(GLOB SCENE_HEADERS ${SOURCE_DIR}/Scene/*.h)
file(GLOB SCENE_SOURCES ${SOURCE_DIR}/Scene/*.cpp)
file(GLOB SCENE_CAMERA_HEADERS ${SOURCE_DIR}/Scene/Camera/*.h)
file(GLOB SCENE_CAMERA_SOURCES ${SOURCE_DIR}/Scene/Camera/*.cpp)
file(GLOB SCENE_GEOMETRY_HEADERS ${SOURCE_DIR}/Scene/Geometry/*.h)
file(GLOB SCENE_GEOMETRY_SOURCES ${SOURCE_DIR}/Scene/Geometry/*.cpp)
file(GLOB SCENE_COMPONENTS_HEADERS ${SOURCE_DIR}/Scene/Components/*.h)
file(GLOB SCENE_COMPONENTS_SOURCES ${SOURCE_DIR}/Scene/Components/*.cpp)
file(GLOB TASK_HEADERS ${SOURCE_DIR}/Task/*.h)
file(GLOB TASK_SOURCES ${SOURCE_DIR}/Task/*.cpp)
file(GLOB THREAD_HEADERS ${SOURCE_DIR}/Thread/*.h ${SOURCE_DIR}/Thread/*.hpp)
file(GLOB THREAD_SOURCES ${SOURCE_DIR}/Thread/*.cpp)
file(GLOB COMMON_PLATFORM_HEADERS ${SOURCE_DIR}/Platform/*.h)
file(GLOB COMMON_PLATFORM_SOURCES ${SOURCE_DIR}/Platform/*.cpp)
file(GLOB UI_HEADERS ${SOURCE_DIR}/UI/*.h)
file(GLOB UI_SOURCES ${SOURCE_DIR}/UI/*.cpp)
if(TARGET_WIN)
    file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Windows/*.h ${SOURCE_DIR}/Platform/XBOX/*.h)
    file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Windows/*.cpp ${SOURCE_DIR}/Platform/XBOX/*.cpp)
elseif(TARGET_ANDROID)
    file(GLOB PLATFORM_HEADERS ${SOURCE_DIR}/Platform/Android/*.h)
    file(GLOB PLATFORM_SOURCES ${SOURCE_DIR}/Platform/Android/*.cpp ${SOURCE_DIR}/Platform/Android/*.c)
endif()
if (BUILD_VULKAN_SDK)
    file(GLOB VULKAN_HEADERS ${SOURCE_DIR}/Renderer/Vulkan/*.h ${SOURCE_DIR}/Renderer/Vulkan/*.inl)
    file(GLOB VULKAN_SOURCES ${SOURCE_DIR}/Renderer/Vulkan/*.cpp)
    file(GLOB RESOURCE_SPIRV_HEADERS ${SOURCE_DIR}/Resource/Decoder/SpirVPatch/*.h)
    file(GLOB RESOURCE_SPIRV_SOURCES ${SOURCE_DIR}/Resource/Decoder/SpirVPatch/*.cpp)
endif()
if(BUILD_DIRECTX_SDK)
    file(GLOB DIRECTX_HEADERS ${SOURCE_DIR}/Renderer/D3D12/*.h)
    file(GLOB DIRECTX_SOURCES ${SOURCE_DIR}/Renderer/D3D12/*.cpp)
endif()
if (IMGUI_SOURCE)
    file(GLOB IMGUI_SOURCES ${SOURCE_DIR}/ThirdParty/imgui/*.h ${SOURCE_DIR}/ThirdParty/imgui/*.cpp)
    file(GLOB IMGUIZMO_SOURCES ${SOURCE_DIR}/ThirdParty/imguizmo/*.h ${SOURCE_DIR}/ThirdParty/imguizmo/*.cpp)
    file(GLOB UI_IMGUI_SOURCES ${SOURCE_DIR}/UI/ImGui/*.h ${SOURCE_DIR}/UI/ImGui/*.cpp)
endif()
file(GLOB SHADER_SOURCES Engine/Data/Shaders/*.h Engine/Data/Shaders/*.hlsli Engine/Data/Shaders/*.hlsl)
set_source_files_properties(${SHADER_SOURCES} PROPERTIES LANGUAGE HLSL)

##############################################################
#Filters
##############################################################
source_group("" FILES ${MAIN_HEADERS} ${MAIN_SOURCES})
source_group("Math" FILES ${MATH_HEADERS} ${MATH_SOURCES})
source_group("Platform" FILES ${COMMON_PLATFORM_HEADERS} ${COMMON_PLATFORM_SOURCES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
source_group("Memory" FILES ${MEMORY_HEADERS} ${MEMORY_SOURCES})
source_group("Utils" FILES ${UTILS_HEADERS} ${UTILS_SOURCES})
source_group("ThirdParty" FILES ${THIRDPARTY_HEADERS} ${THIRDPARTY_SOURCES})
source_group("Events" FILES ${EVENTS_HEADERS} ${EVENTS_SOURCES})
source_group("Events\\Input" FILES ${INPUT_EVENTS_HEADERS} ${INPUT_EVENTS_SOURCES})
source_group("Events\\Game" FILES ${GAME_EVENTS_HEADERS} ${GAME_EVENTS_SOURCES})
source_group("Renderer" FILES ${RENDERER_HEADERS} ${RENDERER_SOURCES})
source_group("RenderTechniques" FILES ${RENDER_TECHNIQUES_HEADERS} ${RENDER_TECHNIQUES_SOURCES})
source_group("Resource" FILES ${RESOURCE_HEADERS} ${RESOURCE_SOURCES})
source_group("Resource\\Decoder" FILES ${RESOURCE_DECODER_HEADERS} ${RESOURCE_DECODER_SOURCES})
source_group("Resource\\Loader" FILES ${RESOURCE_LOADER_HEADERS} ${RESOURCE_LOADER_SOURCES})
source_group("Stream" FILES ${STREAM_HEADERS} ${STREAM_SOURCES})
source_group("Scene" FILES ${SCENE_HEADERS} ${SCENE_SOURCES})
source_group("Scene\\Camera" FILES ${SCENE_CAMERA_HEADERS} ${SCENE_CAMERA_SOURCES})
source_group("Scene\\Geometry" FILES ${SCENE_GEOMETRY_HEADERS} ${SCENE_GEOMETRY_SOURCES})
source_group("Scene\\Components" FILES ${SCENE_COMPONENTS_HEADERS} ${SCENE_COMPONENTS_SOURCES})
source_group("Task" FILES ${TASK_HEADERS} ${TASK_SOURCES})
source_group("Thread" FILES ${THREAD_HEADERS} ${THREAD_SOURCES})
source_group("UI" FILES ${UI_HEADERS} ${UI_SOURCES})
if(BUILD_VULKAN_SDK)
    source_group("Renderer\\Vulkan" FILES ${VULKAN_HEADERS} ${VULKAN_SOURCES})
    source_group("Resource\\Decoder\\SpirVPatch" FILES ${RESOURCE_SPIRV_HEADERS} ${RESOURCE_SPIRV_SOURCES})
endif()
if(BUILD_DIRECTX_SDK)
    source_group("Renderer\\D3D12" FILES ${DIRECTX_HEADERS} ${DIRECTX_SOURCES})
endif()
if (IMGUI_SOURCE)
    source_group("ThirdParty\\imgui" FILES ${IMGUI_SOURCES})
    source_group("ThirdParty\\imguizmo" FILES ${IMGUIZMO_SOURCES})
    source_group("UI\\ImGui" FILES ${UI_IMGUI_SOURCES})
endif()
source_group("Data\\Shaders" FILES ${SHADER_SOURCES})

add_library(${ENGINE_NAME} STATIC
    ${MAIN_HEADERS} ${MAIN_SOURCES}
    ${MATH_HEADERS} ${MATH_SOURCES}
    ${MEMORY_HEADERS} ${MEMORY_SOURCES}
    ${COMMON_PLATFORM_HEADERS} ${COMMON_PLATFORM_SOURCES}
    ${PLATFORM_HEADERS} ${PLATFORM_SOURCES}
    ${UTILS_HEADERS} ${UTILS_SOURCES}
    ${THIRDPARTY_HEADERS} ${THIRDPARTY_SOURCES}
    ${EVENTS_HEADERS} ${EVENTS_SOURCES}
    ${INPUT_EVENTS_HEADERS} ${INPUT_EVENTS_SOURCES}
    ${GAME_EVENTS_HEADERS} ${GAME_EVENTS_SOURCES}
    ${RENDERER_HEADERS} ${RENDERER_SOURCES}
    ${RENDER_TECHNIQUES_HEADERS} ${RENDER_TECHNIQUES_SOURCES}
    ${RESOURCE_HEADERS} ${RESOURCE_SOURCES}
    ${RESOURCE_DECODER_HEADERS} ${RESOURCE_DECODER_SOURCES}
    ${RESOURCE_LOADER_HEADERS} ${RESOURCE_LOADER_SOURCES}
    ${STREAM_HEADERS} ${STREAM_SOURCES}
    ${SCENE_HEADERS} ${SCENE_SOURCES}
    ${SCENE_CAMERA_HEADERS} ${SCENE_CAMERA_SOURCES}
    ${SCENE_GEOMETRY_HEADERS} ${SCENE_GEOMETRY_SOURCES}
    ${SCENE_COMPONENTS_HEADERS} ${SCENE_COMPONENTS_SOURCES}
    ${TASK_HEADERS} ${TASK_SOURCES}
    ${THREAD_HEADERS} ${THREAD_SOURCES}
    ${UI_HEADERS} ${UI_SOURCES}
    $<$<BOOL:BUILD_VULKAN_SDK>:${VULKAN_HEADERS} ${VULKAN_SOURCES} ${RESOURCE_SPIRV_HEADERS} ${RESOURCE_SPIRV_SOURCES}>
    $<$<BOOL:BUILD_DIRECTX_SDK>:${DIRECTX_HEADERS} ${DIRECTX_SOURCES}>
    $<$<BOOL:IMGUI_SOURCE>:${IMGUI_SOURCES} ${IMGUIZMO_SOURCES} ${UI_IMGUI_SOURCES}>
    ${SHADER_SOURCES}
)

target_include_directories(${ENGINE_NAME} BEFORE 
    PRIVATE ${LIB_INCLUDE_DIRECTORY}
    PRIVATE $<$<BOOL:BUILD_VULKAN_SDK>:${VULKAN_SDK_INCLUDE_DIRECTORY}>)


set(BUILD_DEPENDENCIES "")
##############################################################
#Link Libs
##############################################################

#Crc32c	
target_link_libraries(${ENGINE_NAME} crc32c)
set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} crc32c)

#DirectXMath
target_link_libraries(${ENGINE_NAME} DirectXMath)
set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} DirectXMath)

#Assimp	
if(ASSIMP_LIB)
    target_link_libraries(${ENGINE_NAME} assimp)
    set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} assimp)
endif()

#HWCPipe
if(HWCPipe_LIB)
    target_link_libraries(${ENGINE_NAME} hwcpipe)
    set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} hwcpipe)
endif()

#Tracy
if(PROFILER_TRACY_LIB)
    target_link_libraries(${ENGINE_NAME} TracyClient)
    set(BUILD_DEPENDENCIES ${BUILD_DEPENDENCIES} TracyClient)
endif()

# DXCompiler
if(DXCompiler_LIB AND TARGET_WIN)
    message(STATUS "Link DXCompiler dxcompiler.lib")
    target_link_libraries(${ENGINE_NAME} ${ENGINE_PROJECT_DIR}/Engine/External/dxc/lib/x64/dxcompiler.lib)
endif()

#Spirv
if(BUILD_VULKAN_SDK)
    if((IS_DIRECTORY ${SPIRV_FOLDER}/shaderc AND IS_DIRECTORY ${SPIRV_FOLDER}/SPIRV-Cross) OR SPIRV_INSTALL_FROM_GITHUB)
        if(TARGET_ANDROID)
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

set_target_properties(${ENGINE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PLATFORM})
set_target_properties(${ENGINE_NAME} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION ${WIN_SDK_VERSION})
link_directories(${ENGINE_NAME} ${VULKAN_SDK_LIB_DIRECTORY})

#Crc32
message(STATUS "Add Crc32 lib")
set(CRC32_LIB_DIRECTORY Engine/ThirdParty/crc32c)
add_subdirectory(${CRC32_LIB_DIRECTORY})
set_target_properties(crc32c PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CRC32_LIB_DIRECTORY}/${PROJECT_PLATFORM}
    FOLDER ThirdParty/crc32c
    )
set_target_properties(crc32c_arm64 PROPERTIES FOLDER ThirdParty/crc32c)
set_target_properties(crc32c_sse42 PROPERTIES FOLDER ThirdParty/crc32c)
message(STATUS "Crc32 lib added")

#DirectXMath
message(STATUS "Add DirectXMath lib")
set(DIRECTX_MATH_LIB_DIRECTORY Engine/ThirdParty/DirectXMath)
add_subdirectory(${DIRECTX_MATH_LIB_DIRECTORY})
set_target_properties(DirectXMath PROPERTIES FOLDER ThirdParty/DirectXMath)
message(STATUS "DirectXMath lib added")

#Assimp
if(ASSIMP_LIB)
    message(STATUS "Add Assimp lib")
    set(ASSIMP_LIB_DIRECTORY Engine/ThirdParty/assimp)
    add_subdirectory(${ASSIMP_LIB_DIRECTORY})
    if(TARGET_WIN)
        set_target_properties(assimp PROPERTIES 
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSIMP_LIB_DIRECTORY}/code/${PROJECT_PLATFORM}
            FOLDER ThirdParty/assimp
        )
        set_target_properties(zlibstatic PROPERTIES FOLDER ThirdParty/assimp)
        set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER ThirdParty/assimp)
        set_target_properties(uninstall PROPERTIES FOLDER ThirdParty/assimp)

        target_compile_options(assimp PRIVATE -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
        target_compile_options(zlibstatic PRIVATE $<$<CONFIG:DEBUG>:/MTd> $<$<CONFIG:RELEASE>:/MT>)
    elseif(TARGET_ANDROID)
        target_compile_options(assimp PRIVATE -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -frtti)
    endif()
    message(STATUS "Assimp lib added")
endif()

#HWCPipe
if(HWCPipe_LIB)
    message(STATUS "Add HWCPipe lib")
    add_subdirectory(Engine/ThirdParty/HWCPipe)
    message(STATUS "HWCPipe lib added")
endif()

#Tracy
if(PROFILER_TRACY_LIB)
    message(STATUS "Add Tracy lib")
    add_subdirectory(Engine/ThirdParty/tracy)
    set_target_properties(TracyClient PROPERTIES FOLDER ThirdParty/tracy)
    message(STATUS "Tracy lib added")
endif()

#imgui
if(IMGUI_SOURCE)
    message(STATUS "imgui sources added")
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
        set_target_properties(${PROJ} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Examples/${PROJ}/${PROJECT_PLATFORM})
    endforeach(PROJ)
    
    set_target_properties(${BUILD_EXAMPLES} PROPERTIES 
        INCLUDE_DIRECTORIES ${ENGINE_PROJECT_DIR}/${SOURCE_DIR}
        FOLDER Examples
    )
    message(STATUS "----------------")
endif ()
##############################################################

if (BUILD_DEPENDENCIES)
    add_dependencies(${ENGINE_NAME} ${BUILD_DEPENDENCIES})
endif()

##############################################################
#Install external libs
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