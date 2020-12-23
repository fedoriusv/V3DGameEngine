# V3DGameEngine
>V3DEngine 2.0 version<br/>

>Support render: Vulkan, DirectX
>Support platform: Windows, Android

>Required:<br/>
>-- CMake 3<br/>
>-- C++17<br/>
>-- Gradle 3.5<br/>

## Install
```
git clone --recursive https://github.com/fedoriusv/V3DGameEngine.git
```

#### Windows
###### Build in Visual Studio:
>Required:
>-- Visual Studio 16 2019 x64
>-- Python 3 (for building SPIRV)
>To create Visual Studio project execute `project_msvc.bat`

#### Linux
>TODO

#### Android
###### Build in Windows:
>Required:
>-- NDK r20 or later
>-- SDK 26 or later
>-- Ninja
>-- Python 3 (for building SPIRV)

>ANDROID_NDK_HOME - define path to NDK
>ANDROID_HOME - define path to SDK
>For Build and Compile start `project_android.bat`

-------------------------------------------------  

#### General
>Assimp(dae):
>For loading models used Assimp library, to enable it set option to ON in `CMakeLists.txt`

>STB (jpg, png, bmp, tga):<br/>
>Used for loading images, to enable it set option to ON in `CMakeLists.txt`

>gli (ktx, kmg, dds):<br/>
>Used for loading compressed images, to enable it set option to ON in `CMakeLists.txt`

>Profiling can be enabled via adb:
```
adb shell setprop security.perf_harden 0
```
> Performance data is captured using HWCPipe.
> For details on this project and how to integrate it in your pipeline,
> visit: https://github.com/ARM-software/HWCPipe

#### Vulkan
To enable set BUILD_VULKAN_SDK to ON in `CMakeLists.txt` file.
Have to set VK_SDK_PATH in system enviroments.<br/>
For using SPIRV for Vulkan start file:<br/>
Engine\Libs\spirv\install_[windows][android]_platform.bat
Required Python 3

#### DirectX
To enable set BUILD_DIRECTX_SDK to ON in `CMakeLists.txt` file

