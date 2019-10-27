#include "D3D12Debug.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace d3d12
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string D3DDebug::stringError(HRESULT error)
{
    switch (error)
    {
    case DXGI_ERROR_ACCESS_DENIED:
        return "DXGI_ERROR_ACCESS_DENIED";

    case DXGI_ERROR_ACCESS_LOST:
        return "DXGI_ERROR_ACCESS_LOST";

    case DXGI_ERROR_ALREADY_EXISTS:
        return "DXGI_ERROR_ALREADY_EXISTS";

    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
        return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";

    case DXGI_ERROR_DEVICE_HUNG:
        return "DXGI_ERROR_DEVICE_HUNG";

    case DXGI_ERROR_DEVICE_REMOVED:
        return "DXGI_ERROR_DEVICE_REMOVED";

    case DXGI_ERROR_DEVICE_RESET:
        return "DXGI_ERROR_DEVICE_RESET";

    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";

    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
        return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";

    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
        return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";

    case DXGI_ERROR_INVALID_CALL:
        return "DXGI_ERROR_INVALID_CALL";

    case DXGI_ERROR_MORE_DATA:
        return "DXGI_ERROR_MORE_DATA";

    case DXGI_ERROR_NAME_ALREADY_EXISTS:
        return "DXGI_ERROR_NAME_ALREADY_EXISTS";

    case DXGI_ERROR_NONEXCLUSIVE:
        return "DXGI_ERROR_NONEXCLUSIVE";

    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
        return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";

    case DXGI_ERROR_NOT_FOUND:
        return "DXGI_ERROR_NOT_FOUND";

    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
        return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";

    case DXGI_ERROR_REMOTE_OUTOFMEMORY:
        return "DXGI_ERROR_REMOTE_OUTOFMEMORY";

    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
        return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";

    case DXGI_ERROR_SDK_COMPONENT_MISSING:
        return "DXGI_ERROR_SDK_COMPONENT_MISSING";

    case DXGI_ERROR_SESSION_DISCONNECTED:
        return "DXGI_ERROR_SESSION_DISCONNECTED";

    case DXGI_ERROR_UNSUPPORTED:
        return "DXGI_ERROR_UNSUPPORTED";

    case DXGI_ERROR_WAIT_TIMEOUT:
        return "DXGI_ERROR_WAIT_TIMEOUT";

    case DXGI_ERROR_WAS_STILL_DRAWING:
        return "DXGI_ERROR_WAS_STILL_DRAWING";

    default:
        return "unknown";
    }

    return std::string("empty");
}

D3DDebug::D3DDebug() noexcept
    : m_debugDevice(nullptr)
{
}

D3DDebug::~D3DDebug()
{
}

bool D3DDebug::attachDevice(ID3D12Device* device, D3D12_DEBUG_FEATURE flags)
{
    if (m_debugDevice)
    {
        LOG_WARNING("D3DDebug::attachDevice device already has attached");
        return true;
    }

    {
        ASSERT(device, "nullptr");
        HRESULT result = device->QueryInterface(IID_PPV_ARGS(&m_debugDevice));
        if (FAILED(result))
        {
            LOG_ERROR("D3DDebug::attachDevice DeviceQueryInterface is failed. Error %s", D3DDebug::stringError(result).c_str());
            ASSERT(!m_debugDevice, "not nullptr");
            return false;
        }
    }

    {
        HRESULT result = m_debugDevice->SetFeatureMask(flags);
        if (FAILED(result))
        {
            LOG_ERROR("D3DDebug::attachDevice SetFeatureMask is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    return true;
}

bool D3DDebug::report(D3D12_RLDO_FLAGS flags)
{
    ASSERT(m_debugDevice, "nullptr");
    HRESULT result = m_debugDevice->ReportLiveDeviceObjects(flags);
    if (FAILED(result))
    {
        LOG_ERROR("D3DDebug::report ReportLiveDeviceObjects is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12

#endif //D3D_RENDER
