#pragma once

#include "Renderer/DeviceCaps.h"
#include "Utils/Singleton.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DDeviceCaps struct. Vulkan Render side
    */
    struct D3DDeviceCaps : DeviceCaps, utils::Singleton<D3DDeviceCaps>
    {
        D3DDeviceCaps() = default;

        D3DDeviceCaps(const D3DDeviceCaps&) = delete;
        D3DDeviceCaps& operator=(const D3DDeviceCaps&) = delete;

        D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSignatureVersion = {};

        D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureOptions3 = {};
        D3D12_FEATURE_DATA_ARCHITECTURE featureArchitecture = {};
        D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpuVirtualAddress = {};

        D3D_SHADER_MODEL supportedShaderModel = D3D_SHADER_MODEL_5_1;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;

        bool globalComandListAllocator = false;
        bool immediateSubmitUpload = false;
        bool immediateTransition = false;

        u64 memoryConstantBufferSize = 1024 * 64; //64KB
        u32 queryHeapCount = 10000;

    private:

        friend class D3DGraphicContext;

        void initialize(IDXGIAdapter3* adapter, ID3D12Device* device);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
