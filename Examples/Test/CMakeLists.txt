cmake_minimum_required(VERSION 3.0.0)

set (CURRENT_PROJECT "Test")
message("-- " ${CURRENT_PROJECT})

project(${CURRENT_PROJECT})

file(GLOB TEST_HEADERS *.h)
file(GLOB TEST_SOURCES *.cpp)
file(GLOB TEST_SHADERS_GLSL shaders/*.vert shaders/*.frag)
file(GLOB TEST_SHADERS_HLSL shaders/*.vs shaders/*.ps)

source_group(project FILES ${TEST_HEADERS} ${TEST_SOURCES})
source_group("project\\shaders" FILES ${TEST_SHADERS_GLSL} ${TEST_SHADERS_HLSL})

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
endif()

add_executable(${CURRENT_PROJECT} WIN32 ${TEST_HEADERS}	${TEST_SOURCES})

target_link_libraries(${CURRENT_PROJECT} ${ENGINE_NAME})
add_dependencies(${CURRENT_PROJECT} ${ENGINE_NAME})
