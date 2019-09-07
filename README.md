# V3DGameEngine
V3DEngine 2.0 version

Support render: Vulkan, DirectX
Support platform: Windows, Android

Required:
-- CMake 3
-- C++17


## Install
git clone --recursive https://github.com/fedoriusv/V3DGameEngine.git

#### Windows
###### Build in Visual Studio:
Required:
-- Visual Studio 16 2019 x64
-- Python 3 (for building SPIRV)
To create Visual Studio project execute project_msvc.bat

#### Andtoid
###### Build in Windows:
Required:
-- NDK r20 or later
-- SDK 26 or later
-- Ninja
-- Python 3 (for building SPIRV)

ANDROID_NDK - need to define path to NDK
To Build and compile start project_android.bat

-------------------------------------------------  

#### General
Assimp(dae):
For loading models used Assimp library, to enable it set option to ON in CMakeLists.txt

STB (jpg, png, bmp, tga):
Used for loading images, to enable it set option to ON in CMakeLists.txt

gIL (ktx, kmg, dds):
Used for loading compressed images, to enable it set option to ON in CMakeLists.txt

#### Vulkan
To enable set BUILD_VULKAN_SDK to ON in CMakeLists.txt file.
Must be seted VK_SDK_PATH in system enviroments.
For using SPIRV for Vulkan start file:
Engine\Libs\spirv\install_[windows][android]_platform.bat

#### DirectX
To enable set BUILD_DIRECTX_SDK to ON in CMakeLists.txt file


