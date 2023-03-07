#include "D3DDebug.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DGraphicContext.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string D3DDebug::stringError(HRESULT error)
{
    //https://learn.microsoft.com/en-us/windows/win32/direct3d12/d3d12-graphics-reference-returnvalues
    switch (error)
    {
    case E_INVALIDARG:
        return "INVALIDARG";

    case E_OUTOFMEMORY:
        return "OUTOFMEMORY";

    case E_NOTIMPL:
        return "NOTIMPL";

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

#ifdef PLATFORM_WINDOWS
D3DDebug::D3DDebug() noexcept
    : m_debugDevice(nullptr)
{
}

D3DDebug::~D3DDebug()
{
    //The last ptrs inside ID3D12Device
    //SAFE_DELETE(m_debugDevice);
    if (m_debugDevice)
    {
        m_debugDevice->Release();
    }
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
        HRESULT result = device->QueryInterface(&m_debugDevice);
        //device->Release();
        if (FAILED(result))
        {
            LOG_ERROR("D3DDebug::attachDevice DeviceQueryInterface is failed");
            ASSERT(!m_debugDevice, "not nullptr");
            return false;
        }
    }

    if (flags)
    {
        HRESULT result = m_debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS, &flags, sizeof(D3D12_DEBUG_FEATURE));
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
#endif //PLATFORM_WINDOWS

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
D3DDebugLayerMessageCallback::D3DDebugLayerMessageCallback(ID3D12Device* device) noexcept
    : m_infoQueue(nullptr)
    , m_ID(0)
{
    ASSERT(device, "nullptr");
    HRESULT result = device->QueryInterface(&m_infoQueue); //return nullptr for 10.0.20348, maybe need last SDK or OS
    if (FAILED(result))
    {
        LOG_ERROR("D3DDebugLayerMessageCallback::D3DDebugLayerMessageCallback QueryInterface is failed");
        ASSERT(m_infoQueue, "nullptr");
    }
}

D3DDebugLayerMessageCallback::~D3DDebugLayerMessageCallback()
{
    SAFE_DELETE(m_infoQueue);
}

bool D3DDebugLayerMessageCallback::registerMessageCallback(D3D12MessageFunc callbackFunc, D3D12_MESSAGE_CALLBACK_FLAGS flags, void* userData)
{
    ASSERT(m_infoQueue, "nullptr");
    ASSERT(!m_ID, "registered already");
    HRESULT result = m_infoQueue->RegisterMessageCallback(callbackFunc, flags, userData, &m_ID);
    if (FAILED(result))
    {
        LOG_ERROR("D3DDebugLayerMessageCallback::registerMessageCallback RegisterMessageCallback is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    if (m_ID == 0)
    {
        LOG_ERROR("D3DDebugLayerMessageCallback::registerMessageCallback Can't be registered");
        return false;
    }

    return true;
}

bool D3DDebugLayerMessageCallback::unregisterMessageCallback()
{
    ASSERT(m_infoQueue, "nullptr");
    ASSERT(m_ID != 0, "not registered");
    HRESULT result = m_infoQueue->UnregisterMessageCallback(m_ID);
    if (FAILED(result))
    {
        LOG_ERROR("D3DDebugLayerMessageCallback::unregisterMessageCallback UnregisterMessageCallback is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    m_ID = 0;
    return true;
}

void D3DDebugLayerMessageCallback::debugLayersMessageCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID ID, LPCSTR description, void* context)
{
    D3DGraphicContext* dxContext = reinterpret_cast<D3DGraphicContext*>(context);

    switch (severity)
    {
    case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION:
        LOG_FATAL("D3DDebug[CORRUPTION]. ID %d, %s", ID, description);
        break;

    case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR:
        LOG_ERROR("D3DDebug[ERROR]. ID %d, %s", ID, description);
        break;

    case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING:
        LOG_WARNING("D3DDebug[WARNING]. ID %d, %s", ID, description);
        break;

    case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_INFO:
        LOG_INFO("D3DDebug[INFO]. ID %d, %s", ID, description);
        break;

    case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_MESSAGE:
        LOG_DEBUG("D3DDebug[MESSAGE]. ID %d, %s", ID, description);
        break;

    default:
        break;
    }
}
#endif //PLATFORM_WINDOWS

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
