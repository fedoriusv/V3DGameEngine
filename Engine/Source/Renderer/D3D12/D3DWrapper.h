#pragma once

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DWrapper
    {
    public:

#if defined(PLATFORM_WINDOWS)
        static HRESULT CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice) noexcept;
        static HRESULT EnableExperimentalFeatures(UINT NumFeatures, const IID* pIIDs, void* pConfigurationStructs, UINT* pConfigurationStructSizes) noexcept;
        static HRESULT GetDebugInterface(REFIID riid, void** ppvDebug) noexcept;

#elif defined(PLATFORM_XBOX)
        static HRESULT CreateDevice(IGraphicsUnknown* pAdapter, const D3D12XBOX_CREATE_DEVICE_PARAMETERS* pParameters, REFIID riid, void** ppDevice) noexcept;
#endif //PLATFORM
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER

