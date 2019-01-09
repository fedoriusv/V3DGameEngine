@echo off
set VS_VERSION="Visual Studio 15 2017 Win64"

if not exist Project\Win64 mkdir Project\Win64
cd Project\Win64
cmake -DCOMPILER_MSVC=ON -G%VS_VERSION% ../..

pause