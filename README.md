# V3DGameEngine
V3DEngine 2.0 version

Support render: Vulkan, DirectX
Support platform: Windows, Android

Required:
-- C++17

## Install
git clone --recursive https://github.com/fedoriusv/V3DGameEngine.git

#### Windows
To create Visual Studion project execute project_msvc.bat

##### Vulkan
To enable set BUILD_VULKAN_SDK to ON in CMakeLists.txt file
For using SPIRV for Vulkan start file:
Engine\Libs\spirv\install.bat

##### DirectX
To enable set BUILD_DIRECTX_SDK to ON in CMakeLists.txt file


#### Andtoid
On Windows:
ANDROID_NDK - should be definded

To Build and compile start project_android.bat