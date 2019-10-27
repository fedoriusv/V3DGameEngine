#pragma once

#include "Common.h"
#include "Utils/Singleton.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DDebug final class. DirectX Render side
    */
    class D3DDebug final : public utils::Singleton<D3DDebug>
    {
    public:

        D3DDebug() noexcept;
        ~D3DDebug();

        static std::string stringError(HRESULT error);

        bool attachDevice(ID3D12Device* device, D3D12_DEBUG_FEATURE flags);
        bool report(D3D12_RLDO_FLAGS flags);

    private:

        ComPtr<ID3D12DebugDevice> m_debugDevice;
    };

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER