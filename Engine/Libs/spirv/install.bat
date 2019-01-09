@echo off

REM VS_VERSION set inside project_msvc*.bat
REM vcvarsall.bat find it here: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build

if %VS_VERSION% == [] set VS_VERSION="Visual Studio 15 2017 Win64"

if not exist shaderc git clone https://github.com/google/shaderc

cd shaderc/third_party
REM  git clone https://github.com/google/googletest.git
if not exist glslang git clone https://github.com/google/glslang.git
if not exist spirv-tools (
git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-tools/external/spirv-headers
)

cd ../

cmake -DSHADERC_SKIP_TESTS=ON -G%VS_VERSION%
REM cmake --build . --config {Debug}
REM cmake --build . --config {Realese}

@echo on
call vcvarsall.bat x64
echo "Shaderc builds started..." 
devenv "%CD%\shaderc.sln" /build "Debug|x64"
devenv "%CD%\shaderc.sln" /build "Release|x64"
echo "Shaderc builds finished..." 
echo "Shaderc All builds completed."
cd ../

@echo off
if not exist SPIRV-Cross git clone https://github.com/KhronosGroup/SPIRV-Cross.git
cd SPIRV-Cross
cmake -DCMAKE_CXX_FLAGS_DEBUG="/Zi /MTd" -DCMAKE_CXX_FLAGS_RELEASE="/MT" -G%VS_VERSION%
@echo on
call vcvarsall.bat x64
echo "SPIRV-Cross builds started..." 
devenv "%CD%\SPIRV-Cross.sln" /build "Debug|x64"
devenv "%CD%\SPIRV-Cross.sln" /build "Release|x64"
echo "SPIRV-Cross builds finished..." 
echo "SPIRV-Cross All builds completed."
cd ../

pause