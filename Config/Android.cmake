#Android Config
cmake_minimum_required(VERSION 3.10)

if(NOT TARGET_ANDROID)
    message(FATAL_ERROR "Platform Android must be enabled for this config")
endif()

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/scripts/android.toolchain.cmake" CACHE STRING "The Android toolchain file")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")
#set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti -llog -landroid")

add_definitions(-D_ANDROID -DUNICODE -D_UNICODE)

message(STATUS "---------------------------")
message(STATUS "Toolchain       : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Android ABI     : ${ANDROID_ABI}")
message(STATUS "Android platform: ${ANDROID_PLATFORM}")
message(STATUS "Android NDK     : ${ANDROID_NDK}")
message(STATUS "Build Type      : ${CMAKE_BUILD_TYPE}")
#message(STATUS "Compile include ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "---------------------------")


#Configure Gradle
set(GRADLE_BUILD_FILE Config/gradle/build.gradle.template)
set(GRADLE_PROPERTIES_FILE Config/gradle/gradle.properties.template)
set(ANDROID_MANIFEST_FILE Config/gradle/AndroidManifest.xml.template)

if (BUILD_EXAMPLES)
    file(READ "ExamplesList.txt" STRING_BUILD_EXAMPLES)
    string(REPLACE "\n" ";" BUILD_EXAMPLES ${STRING_BUILD_EXAMPLES})
    message(STATUS "Gradle Configuring:")
    foreach (PROJ ${BUILD_EXAMPLES})
        set(GRADLE_BUILD_OUTPUT_DIR Examples/${PROJ})
        message(STATUS "Gradle configuring ${GRADLE_BUILD_OUTPUT_DIR}")
        
        set(TEMPLATE_SDK_VERSION ${ANDROID_PLATFORM_LEVEL})
        set(TEMPLATE_PROJECT_NAME ${PROJ})
        set(TEMPLATE_NDK_ABI_LIST "'${ANDROID_ABI}'")
        
        configure_file(${GRADLE_BUILD_FILE} ${GRADLE_BUILD_OUTPUT_DIR}/build.gradle)
        configure_file(${GRADLE_PROPERTIES_FILE} ${GRADLE_BUILD_OUTPUT_DIR}/gradle.properties)
        configure_file(${ANDROID_MANIFEST_FILE} ${GRADLE_BUILD_OUTPUT_DIR}/AndroidManifest.xml)
    endforeach(PROJ)
endif ()

