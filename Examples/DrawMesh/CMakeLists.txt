cmake_minimum_required(VERSION 3.15)

set(CURRENT_PROJECT "DrawMesh")
message("-- " ${CURRENT_PROJECT})

project(${CURRENT_PROJECT})

file(GLOB PROJECT_HEADERS *.h)
file(GLOB PROJECT_SOURCES *.cpp)

file(GLOB SHADER_FILES "data/shaders/*")
set_source_files_properties(${SHADER_FILES} PROPERTIES LANGUAGE GLSL)

source_group("project" FILES ${PROJECT_HEADERS} ${PROJECT_SOURCES})
source_group("project\\shaders" FILES ${SHADER_FILES})

if(TARGET_WIN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
    add_executable(${CURRENT_PROJECT} WIN32 ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${SHADER_FILES})
elseif (TARGET_ANDROID)
    add_library(${CURRENT_PROJECT} SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
    target_link_libraries(${CURRENT_PROJECT} log android)
endif()

target_link_libraries(${CURRENT_PROJECT} ${ENGINE_NAME})
add_dependencies(${CURRENT_PROJECT} ${ENGINE_NAME})
