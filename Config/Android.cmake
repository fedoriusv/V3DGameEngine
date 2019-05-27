#Android arm64-v8a Config

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/scripts/android.toolchain.cmake" CACHE STRING "The Android toolchain file")

# {armeabi-v7a,arm64-v8a}
#set(ANDROID_ABI "arm64-v8a" CACHE STRING "The Android ABI to build for")
#set(ANDROID_PLATFORM "android-26" CACHE STRING "The Android Platform to build for")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -I$ENV{ANDROID_NDK}/sources/android/native_app_glue")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -I$ENV{ANDROID_NDK}/sources/android/native_app_glue")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti -llog -landroid")
#set(CMAKE_DEBUG_POSTFIX "_d")

message(STATUS "Toolchain ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Android ABI ${ANDROID_ABI}")
message(STATUS "Android platform ${ANDROID_PLATFORM}")
message(STATUS "Android NDK ${ANDROID_NDK}")
message(STATUS "Build Type ${CMAKE_BUILD_TYPE}")
#message(STATUS "Compile include ${CMAKE_CXX_FLAGS_DEBUG}")