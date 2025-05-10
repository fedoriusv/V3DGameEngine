@echo off

setlocal
set VS_VERSION="Visual Studio 17 2022"

REM shaderc
if not exist shaderc (
	git clone https://github.com/google/shaderc.git

	cd shaderc/third_party
	REM  git clone https://github.com/google/googletest.git
	git clone https://github.com/KhronosGroup/glslang.git glslang
	git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-tools/external/spirv-headers
	cd ../../
)
=
cd shaderc
mkdir build_windows
cd build_windows
cmake -G%VS_VERSION% -A x64 -DSHADERC_SKIP_TESTS=ON -DSHADERC_SKIP_EXAMPLES=ON -DSHADERC_ENABLE_SHARED_CRT=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ../
cmake --build . --config Debug
cmake --build . --config Release
cd ../
cd ../

REM SPIRV-Cross
if not exist SPIRV-Cross (
	git clone https://github.com/KhronosGroup/SPIRV-Cross.git
)	

cd SPIRV-Cross
mkdir build_windows
cd build_windows
cmake -G%VS_VERSION% -A x64 -DSPIRV_CROSS_STATIC=ON -DCMAKE_CXX_FLAGS_DEBUG="/MTd" -DCMAKE_CXX_FLAGS_RELEASE="/MT" ../
cmake --build . --config Debug
cmake --build . --config Release
cd ../
cd ../

pause