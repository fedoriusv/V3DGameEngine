@echo off

setlocal
set command=all

if "%ANDROID_NDK%" == "" (
	echo Error. Variable ANDROID_NDK is NOT defined
	pause
	exit
)

if "%1" == "" goto all

if "%1" == "--help" (
    goto help
)

if "%1" == "--clean" (
    set command=single
    goto clean
)
    
if "%1" == "--prepare" (
    set command=single
    goto prepare
)
    
if "%1" == "--build" (
    set command=single
    goto build
)

:help
    echo Build Android
    echo --clean          Clean build folder
    echo --prepare        Generate ninja cmake project
    echo --build          Build Android .so file
    echo --pack           Pack resources
    goto end

:all
:clean
    echo Clean
    if exist Project\Android (
        rmdir /S /Q Project\Android
	)
    if "%command%" == "single" goto end
   
:prepare
    echo Prepare
    if not exist Project\Android (
        mkdir Project\Android
    )
    cd Project\Android

    set C_ANDROID_TOOLCHAIN=%ANDROID_NDK%\build\cmake\android.toolchain.cmake
    set C_ANDROID_NDK=%ANDROID_NDK%
    set C_ANDROID_ABI=arm64-v8a
    set C_ANDROID_PLATFORM=android-26
    set C_BUILD_TYPE=Debug

    cmake -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_NDK=%C_ANDROID_NDK% -DANDROID_ABI=%C_ANDROID_ABI% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DCOMPILER_CLANG=ON -DTARGET_ANDROID=ON -DCMAKE_MAKE_PROGRAM=Ninja -GNinja -DCMAKE_BUILD_TYPE=%C_BUILD_TYPE% -DCRC32C_BUILD_TESTS=OFF -DCRC32C_BUILD_BENCHMARKS=OFF -DCRC32C_USE_GLOG=OFF -DCRC32C_INSTALL=OFF -DASSIMP_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ../..
    cd ..\..
    if "%command%" == "single" goto end
    
:build
    echo Build
    if not exist Project\Android (
        echo Error. Start prepare first
        goto end
    )
    cd Project\Android
    
    cmake --build .
    cd ..\..
    if "%command%" == "single" goto end
    
:pack
    echo Pack
    if not exist Project\Android (
        echo Error. Start prepare first
        goto end
    )
    cd Project\Android
    
rem    mkdir "assets"
rem	   xcopy "..\data\*.*" "assets" /SY

    cd ..\..
    if "%command%" == "single" goto end
    
    
:end
if "%command%" == "all" pause