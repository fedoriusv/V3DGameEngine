#include "D3D12Wrapper.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace d3d12
{

HRESULT D3DWrapper::CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice) noexcept
{
    return D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
}

HRESULT D3DWrapper::GetDebugInterface(REFIID riid, void** ppvDebug) noexcept
{
    return  D3D12GetDebugInterface(riid, ppvDebug);
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER