#pragma once

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

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

        static HRESULT CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice) noexcept;
        static HRESULT GetDebugInterface(REFIID riid, void** ppvDebug) noexcept;

        //TODO
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER

