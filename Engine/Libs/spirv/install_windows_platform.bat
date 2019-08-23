@echo off

REM vcvarsall.bat find it here: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build

setlocal
set VS_VERSION="Visual Studio 16 2019"

if not exist shaderc (
	git clone https://github.com/google/shaderc.git

	cd shaderc/third_party
	REM  git clone https://github.com/google/googletest.git
	git clone https://github.com/KhronosGroup/glslang.git glslang
	git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-tools/external/spirv-headers
	cd ../../
)

cd shaderc

mkdir build_windows
cd build_windows
cmake -G%VS_VERSION% -A x64 -DSHADERC_SKIP_TESTS=ON ../
cmake --build . --config Debug
cmake --build . --config Release
cd ../

cd ../

REM @echo on
REM call vcvarsall.bat x64
REM echo "Shaderc builds started..." 
REM devenv "%CD%\shaderc.sln" /build "Debug|x64"
REM devenv "%CD%\shaderc.sln" /build "Release|x64"
REM echo "Shaderc builds finished..." 
REM echo "Shaderc All builds completed."
REM cd ../
REM @echo off

if not exist SPIRV-Cross (
	git clone https://github.com/KhronosGroup/SPIRV-Cross.git
)	

cd SPIRV-Cross

mkdir build_windows
cd build_windows
cmake -G%VS_VERSION% -A x64 -DCMAKE_CXX_FLAGS_DEBUG="/Zi /MTd" -DCMAKE_CXX_FLAGS_RELEASE="/MT" ../
cmake --build . --config Debug
cmake --build . --config Release
cd ../

cd ../

REM @echo on
REM call vcvarsall.bat x64
REM echo "SPIRV-Cross builds started..." 
REM devenv "%CD%\SPIRV-Cross.sln" /build "Debug|x64"
REM devenv "%CD%\SPIRV-Cross.sln" /build "Release|x64"
REM echo "SPIRV-Cross builds finished..." 
REM echo "SPIRV-Cross All builds completed."
REM cd ../
REM @echo off

pause