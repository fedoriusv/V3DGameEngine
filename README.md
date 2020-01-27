# V3DGameEngine
V3DEngine 2.0 version<br/>

Support render: Vulkan,DirectX<br/>
Support platform: Windows, Android<br/>

Required:<br/>
-- CMake 3<br/>
-- C++17<br/>

## Install
git clone --recursive https://github.com/fedoriusv/V3DGameEngine.git


#### Windows
###### Build in Visual Studio:
Required:<br/>
-- Visual Studio 16 2019 x64<br/>
-- Python 3 (for building SPIRV)<br/>
To create Visual Studio project execute project_msvc.bat<br/>

#### Android
###### Build in Windows:
Required:<br/>
-- NDK r20 or later<br/>
-- SDK 26 or later<br/>
-- Ninja<br/>
-- Python 3 (for building SPIRV)<br/>

ANDROID_NDK - define path to NDK<br/>
ANDROID_HOME - define path to SDK<br/>
For Build and Compile start project_android.bat<br/>

-------------------------------------------------  

#### General
Assimp(dae):<br/>
For loading models used Assimp library, to enable it set option to ON in CMakeLists.txt<br/>

STB (jpg, png, bmp, tga):<br/>
Used for loading images, to enable it set option to ON in CMakeLists.txt<br/>

gli (ktx, kmg, dds):<br/>
Used for loading compressed images, to enable it set option to ON in CMakeLists.txt<br/>

#### Vulkan
To enable set BUILD_VULKAN_SDK to ON in CMakeLists.txt file.<br/>
Have to set VK_SDK_PATH in system enviroments.<br/>
For using SPIRV for Vulkan start file:<br/>
Engine\Libs\spirv\install_[windows][android]_platform.bat<br/>


#### DirectX
To enable set BUILD_DIRECTX_SDK to ON in CMakeLists.txt file<br/>



