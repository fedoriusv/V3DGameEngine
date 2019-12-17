cmake_minimum_required(VERSION 3.0.0)

set (CURRENT_PROJECT "DrawScene")
message("-- " ${CURRENT_PROJECT})

project(${CURRENT_PROJECT})

file(GLOB TEST_HEADERS *.h)
file(GLOB TEST_SOURCES *.cpp)
file(GLOB TEST_SHADERS "data/shaders/*.frag" "data/shaders/*.vert")

source_group("project" FILES ${TEST_HEADERS} ${TEST_SOURCES})
source_group("project\\shaders" FILES ${TEST_SHADERS})

if(TARGET_WIN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
	add_executable(${CURRENT_PROJECT} WIN32 ${TEST_HEADERS}	${TEST_SOURCES})
elseif (TARGET_ANDROID)
	add_library(${CURRENT_PROJECT} SHARED ${TEST_HEADERS} ${TEST_SOURCES})
endif()

target_link_libraries(${CURRENT_PROJECT} ${ENGINE_NAME})
add_dependencies(${CURRENT_PROJECT} ${ENGINE_NAME})
