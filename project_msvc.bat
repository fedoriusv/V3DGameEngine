@echo off

setlocal
set VS_VERSION="Visual Studio 17 2022"
set VS_WIN_SDK_VERSION=10.0.22621.0

if not exist Project\Win64 mkdir Project\Win64
cd Project\Win64

cmake -G%VS_VERSION% -A x64 -DCOMPILER_MSVC=ON -DTARGET_WIN=ON -DWIN_SDK_VERSION=%VS_WIN_SDK_VERSION% ../..

pause