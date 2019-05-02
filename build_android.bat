@echo off

if not exist Project\Android mkdir Project\Android
cd Project\Android

set ANDROID_TOOLCHAIN=%ANDROID_HOME%\build\cmake\android.toolchain.cmake

cmake --build -DCMAKE_TOOLCHAIN_FILE=%ANDROID_TOOLCHAIN%

pause