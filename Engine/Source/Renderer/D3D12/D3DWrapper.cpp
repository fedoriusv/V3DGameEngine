#include "D3DWrapper.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace dx3d
{
#if defined(PLATFORM_WINDOWS)
HRESULT D3DWrapper::CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice) noexcept
{
    return D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
}

HRESULT D3DWrapper::EnableExperimentalFeatures(UINT NumFeatures, const IID* pIIDs, void* pConfigurationStructs, UINT* pConfigurationStructSizes) noexcept
{
    return D3D12EnableExperimentalFeatures(NumFeatures, pIIDs, pConfigurationStructs, pConfigurationStructSizes);
}

HRESULT D3DWrapper::GetDebugInterface(REFIID riid, void** ppvDebug) noexcept
{
    return  D3D12GetDebugInterface(riid, ppvDebug);
}

#elif defined(PLATFORM_XBOX)
HRESULT D3DWrapper::CreateDevice(IGraphicsUnknown* pAdapter, const D3D12XBOX_CREATE_DEVICE_PARAMETERS* pParameters, REFIID riid, void** ppDevice) noexcept
{
    return D3D12XboxCreateDevice(pAdapter, pParameters, riid, ppDevice);
}
#endif //PLATFORM

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
