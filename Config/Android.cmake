#Android arm64-v8a Config

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/scripts/android.toolchain.cmake" CACHE STRING "The Android toolchain file")

# {armeabi-v7a,arm64-v8a}
#set(ANDROID_ABI "arm64-v8a" CACHE STRING "The Android ABI to build for")
#set(ANDROID_PLATFORM "android-26" CACHE STRING "The Android Platform to build for")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++14")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14")
elseif()
	message(FATAL_ERROR "Compiler not found")
endif()
set(CMAKE_DEBUG_POSTFIX "_d")

message(STATUS "Toolchain ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Android ABI ${ANDROID_ABI}")
message(STATUS "Android platform ${ANDROID_PLATFORM}")
message(STATUS "Android NDK ${ANDROID_NDK}")