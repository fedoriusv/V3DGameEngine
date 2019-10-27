#include "D3D12Fence.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include <synchapi.h>

#   include "D3D12GraphicContext.h"
#   include "D3D12Debug.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3DFence::D3DFence(ID3D12Device* device, u64 value) noexcept
    : m_device(device)
    , m_fenceEvent(NULL)
    , m_fence(nullptr)
{
    LOG_DEBUG("D3DFence::D3DFence constructor %llx", this);

    {
        HRESULT result = m_device->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(result))
        {
            LOG_ERROR("D3DFence::D3DFence CreateFence is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    {
        m_fenceEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (m_fenceEvent == NULL)
        {
            HRESULT result = HRESULT_FROM_WIN32(GetLastError());
            LOG_ERROR("D3DFence::D3DFence CreateFence is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    m_value = m_fence->GetCompletedValue();
}

D3DFence::~D3DFence()
{
    LOG_DEBUG("D3DFence::~D3DFence destructor %llx", this);

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    if (m_fence)
    {
        m_fence->Release();
        m_fence = nullptr;
    }
}

bool D3DFence::signal(u64 value)
{
    ASSERT(m_fence, "nullptr");
    HRESULT result = m_fence->Signal(value);
    if (FAILED(result))
    {
        LOG_ERROR("D3DFence::signal Signal is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    m_value = m_fence->GetCompletedValue();
    return true;
}


bool D3DFence::wait(u64 value)
{
    m_value = m_fence->GetCompletedValue();

    ASSERT(m_fence, "nullptr");
    if (!D3DFence::completed(value))
    {
        ASSERT(m_fenceEvent, "nullptr");
        HRESULT result = m_fence->SetEventOnCompletion(value, m_fenceEvent);
        if (FAILED(result))
        {
            LOG_ERROR("D3DFence::wait SetEventOnCompletion is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }

        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
#if DEBUG
        if (m_fence->GetCompletedValue() < value)
        {
            ASSERT(false, "must be completed");
            return false;
        }
#endif
    }

    return true;
}

void D3DFence::reset(u64 value)
{
    ASSERT(m_fence, "nullptr");
    {
        HRESULT result = m_fence->Signal(value);
        if (FAILED(result))
        {
            LOG_ERROR("D3DFence::reset Signal is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    {
        bool result = ResetEvent(m_fenceEvent);
        ASSERT(result, "error");
    }
}

bool D3DFence::completed(u64 value)
{
    m_value = m_fence->GetCompletedValue();

    ASSERT(m_fence, "nullptr");
    const u64 fenceStatus = m_fence->GetCompletedValue();
    if (fenceStatus > value)
    {
        return true;
    }

    return false;
}

ID3D12Fence* D3DFence::getHandle() const
{
    ASSERT(m_fence, "nullptr");
    return m_fence;
}


} //namespace d3d12
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER