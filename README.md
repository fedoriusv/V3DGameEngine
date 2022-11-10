# V3DGameEngine
V3DEngine 2.0 version<br/>

Support render: Vulkan, DirectX<br/>
Support platform: Windows, Android, XBOX<br/>

>Required:<br/>
>-- CMake 3.10 or later<br/>
>-- C++20<br/>

## Install
```
git clone --recursive https://github.com/fedoriusv/V3DGameEngine.git
```

#### Windows
###### Build in Visual Studio:
>Required:<br/>
>-- Visual Studio 17 2022<br/>
>-- Windows SDK 10.0.20348 or later(for DirectX)<br/>
>-- Python 3 (for building SPIRV)<br/>

To create Visual Studio project execute `project_msvc.bat`<br/>

#### Linux
TODO<br/>

#### Android
###### Build in Windows:
>Required:<br/>
>-- NDK r20 or later<br/>
>-- SDK 26 or later<br/>
>-- Ninja<br/>
>-- Gradle 3.5<br/>
>-- Python 3 (for building SPIRV)<br/>

`ANDROID_NDK_HOME` - define path to NDK<br/>
`ANDROID_HOME` - define path to SDK<br/>
For Build and Compile start `project_android.bat`<br/>

To enable profiling HWCPipe set option to ON in `CMakeLists.txt`<br/>
Profiling can be enabled via adb:
```
adb shell setprop security.perf_harden 0
```
visit: https://github.com/ARM-software/HWCPipe

-------------------------------------------------  

#### General
Assimp (Release3.1):<br/>
For loading models ang geometry. Supported formats: dae
Set option to `ON` in `CMakeLists.txt` to enable<br/>

STB (0.9.5.3):<br/>
Uses for loading images. Supported formats: jpg, png, bmp, tga<br/>
Set option to `ON` in `CMakeLists.txt` to enable<br/>

gli (0.8.2)<br/>
Uses for loading compressed images, Supported formatd: ktx, kmg, dds<br/>
Set option to `ON` in `CMakeLists.txt` to enable<br/>


#### Vulkan
To enable vulkan API set `BUILD_VULKAN_SDK` to ON in `CMakeLists.txt` file.
`VK_SDK_PATH` must be setted in system enviroments.<br/>
For using SPIRV for Vulkan start file:<br/>
Engine\Libs\spirv\install_[windows][android]_platform.bat<br/>
>Required:<br/>
>-- Vulkan SDK: https://vulkan.lunarg.com/<br/>
>-- Python 3<br/>


#### DirectX
To enable set `BUILD_DIRECTX_SDK` to ON in `CMakeLists.txt` file.<br/>
For using custom DirectXShaderCompiler(DXC) set option to ON in `CMakeLists.txt`
