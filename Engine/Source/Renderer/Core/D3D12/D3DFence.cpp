#include "D3DFence.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include <synchapi.h>

#include "D3DDebug.h"
#include "D3DGraphicContext.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DFence::D3DFence(ID3D12Device* device, u64 value) noexcept
    : m_device(device)

    , m_fenceEvent(NULL)
    , m_fence(nullptr)

    , m_value(value)
{
    LOG_DEBUG("D3DFence::D3DFence constructor %llx", this);

    {
        HRESULT result = m_device->CreateFence(value, D3D12_FENCE_FLAG_NONE, DX_IID_PPV_ARGS(&m_fence));
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
}

D3DFence::~D3DFence()
{
    LOG_DEBUG("D3DFence::~D3DFence destructor %llx", this);

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
    
    SAFE_DELETE(m_fence)
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

    return true;
}

bool D3DFence::signal(ID3D12CommandQueue* queue)
{
    m_value = m_fence->GetCompletedValue();
    u64 waitValue = m_value + 1;

    ASSERT(m_fence && queue, "nullptr");
    HRESULT result = queue->Signal(m_fence, waitValue);
    if (FAILED(result))
    {
        LOG_ERROR("D3DFence::signal Signal is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    return true;
}


bool D3DFence::wait(u64 value)
{
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

bool D3DFence::wait()
{
    ASSERT(m_fence, "nullptr");
    if (!D3DFence::completed(m_value))
    {
        ASSERT(m_fenceEvent, "nullptr");
        HRESULT result = m_fence->SetEventOnCompletion(m_value + 1, m_fenceEvent);
        if (FAILED(result))
        {
            LOG_ERROR("D3DFence::wait SetEventOnCompletion is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }

        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
#if DEBUG
        if (m_fence->GetCompletedValue() < m_value)
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
    ASSERT(m_fence, "nullptr");
    const u64 fenceStatus = m_fence->GetCompletedValue();
    if (fenceStatus > value)
    {
        return true;
    }

    return false;
}

bool D3DFence::completed()
{
    ASSERT(m_fence, "nullptr");
    const u64 fenceStatus = m_fence->GetCompletedValue();
    ASSERT(fenceStatus != UINT64_MAX, "fail");
    if (m_value < fenceStatus)
    {
        return true;
    }

    return false;
}

u64 D3DFence::incrementValue()
{
    u64 value = m_value.fetch_add(1, std::memory_order_relaxed);
    return value;
}

u64 D3DFence::getFenceValue() const
{
    ASSERT(m_fence, "nullptr");
    u64 value = m_fence->GetCompletedValue();
    return value;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER