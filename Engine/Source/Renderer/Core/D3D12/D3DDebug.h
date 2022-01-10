#pragma once

#include "Common.h"
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
    * @brief D3DDebug final class. DirectX Render side
    */
    class D3DDebug final : public utils::Singleton<D3DDebug>
    {
    public:

        static std::string stringError(HRESULT error);

#ifdef PLATFORM_WINDOWS
        D3DDebug() noexcept;
        ~D3DDebug();

        bool attachDevice(ID3D12Device* device, D3D12_DEBUG_FEATURE flags);
        bool report(D3D12_RLDO_FLAGS flags);

    private:

        ID3D12DebugDevice2* m_debugDevice;
#endif //PLATFORM_WINDOWS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
    /**
    * @brief D3DDebugLayerMessageCallback class. DirectX Render side.
    * Callback system for validation layers
    */
    class D3DDebugLayerMessageCallback final
    {
    public:

        D3DDebugLayerMessageCallback(ID3D12Device* device) noexcept;
        D3DDebugLayerMessageCallback(const D3DDebugLayerMessageCallback&) = delete;
        ~D3DDebugLayerMessageCallback();

        bool registerMessageCallback(D3D12MessageFunc callbackFunc, D3D12_MESSAGE_CALLBACK_FLAGS flags, void* userData = nullptr);
        bool unregisterMessageCallback();

        static void debugLayersMessageCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID ID, LPCSTR description, void* context);

    private:

        ID3D12InfoQueue1* m_infoQueue;
        DWORD m_ID;
    };
#endif //PLATFORM_WINDOWS

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER