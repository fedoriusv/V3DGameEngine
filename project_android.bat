@echo off

setlocal
if "%ANDROID_NDK%" == "" (
	echo Error. Variable ANDROID_NDK doesn't defined
	pause
	exit
) 

set C_ANDROID_TOOLCHAIN=%ANDROID_NDK%\build\cmake\android.toolchain.cmake
set C_ANDROID_NDK=%ANDROID_NDK%
set C_ANDROID_ABI=arm64-v8a
set C_ANDROID_PLATFORM=android-26
set C_BUILD_TYPE=Debug

set command=all
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

if "%1" == "--pack" (
    if "%2" == "" (
        echo Error. Name of pack is empty or not exist
        goto end
    )
    set command=single
    goto pack
)

if "%1" == "--install" (
if "%2" == "" (
        echo Error. Name of pack is empty or not exist
        goto end
    )
    set command=single
    goto install
)

:help
    echo Build Android
    echo --clean          Clean build folder
    echo --prepare        Generate ninja cmake project
    echo --build          Build Android .so file
    echo --pack "name"    Pack resources
    echo --install "name" Install APK to device
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

    cmake -GNinja -DCMAKE_MAKE_PROGRAM=Ninja -DCMAKE_TOOLCHAIN_FILE=%C_ANDROID_TOOLCHAIN% -DANDROID_NDK=%C_ANDROID_NDK% -DANDROID_ABI=%C_ANDROID_ABI% -DANDROID_PLATFORM=%C_ANDROID_PLATFORM% -DCMAKE_BUILD_TYPE=%C_BUILD_TYPE% -DCOMPILER_CLANG=ON -DTARGET_ANDROID=ON  -DCRC32C_BUILD_TESTS=OFF -DCRC32C_BUILD_BENCHMARKS=OFF -DCRC32C_USE_GLOG=OFF -DCRC32C_INSTALL=OFF -DASSIMP_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ../..
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
    
    echo Examples list:
    for /F %%i in (ExamplesList.txt) do @echo -- %%i
    goto end

:pack
    echo Pack
    if not exist Project\Android\Examples\%2 (
        echo Error. Project has not found
        goto end
    )
    
    xcopy /Y Examples\%2\AndroidManifest.xml Project\Android\Examples\%2
    xcopy /Y Examples\%2\build.gradle Project\Android\Examples\%2
    xcopy /Y Examples\%2\gradle.properties Project\Android\Examples\%2
    xcopy /Y /F /S Examples\%2\res Project\Android\Examples\%2\res\
    
    if exist Examples\%2\data (
        xcopy /Y /F /S Examples\%2\data Project\Android\Examples\%2\assets\
    )
    
    if "%C_BUILD_TYPE%" == "Debug" (
        xcopy /Y "Project\Android\Examples\%2\lib%2d.so" "Project\Android\Examples\%2\libs\%C_ANDROID_ABI%\lib%2.so*"
    ) else (
        xcopy /Y "Project\Android\Examples\%2\lib%2.so" "Project\Android\Examples\%2\libs\%C_ANDROID_ABI%\lib%2.so*"
    )
    
    rem fix build error
    set ANDROID_NDK=""
    set ANDROID_NDK_HOME=""
    
    cd Config
    call gradlew.bat -p ../Project/Android/Examples/%2 build
    cd ..
    goto end
    
:install
    echo Installing APK..
    if "%C_BUILD_TYPE%" == "Debug" (
        call adb install -r Project/Android/Examples/%2/build/outputs/apk/%2-debug.apk
    ) else (
        call adb install -r Project/Android/Examples/%2/build/outputs/apk/%2.apk
    )
    goto end
    
:end
if "%command%" == "all" pause