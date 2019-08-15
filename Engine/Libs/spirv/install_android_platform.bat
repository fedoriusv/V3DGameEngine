@echo off

setlocal

set C_ANDROID_TOOLCHAIN=%ANDROID_NDK%\build\cmake\android.toolchain.cmake
set C_ANDROID_ABI=arm64-v8a
set C_ANDROID_PLATFORM=android-26

if not exist shaderc (
	git clone https://github.com/google/shaderc.git

	cd shaderc/third_party
	REM  git clone https://github.com/google/googletest.git
	git clone https://github.com/KhronosGroup/glslang.git glslang
	git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-tools/external/spirv-headers
	cd ../..
)

cd shaderc

mkdir build_android\Debug
cd build_android\Debug
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DANDROID_ABI=%C_ANDROID_ABI% -DSHADERC_SKIP_TESTS=ON ../..
ninja
cd ../..

mkdir build_android\Release
cd build_android\Release
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DANDROID_ABI=%C_ANDROID_ABI% -DSHADERC_SKIP_TESTS=ON ../..
REM -DCMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE} -flto -Qunused-arguments"
ninja
cd ../..

cd ../
dir

pause

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

mkdir build_android\Debug
cd build_android\Debug
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DANDROID_ABI=%C_ANDROID_ABI% ../..
ninja
cd ../..

mkdir build_android\Release
cd build_android\Release
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DANDROID_ABI=%C_ANDROID_ABI% ../..
ninja
cd ../..

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