#include "D3D12CommandListManager.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include "D3D12Debug.h"
#   include "D3D12Fence.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3DCommandListManager::D3DCommandListManager(ID3D12Device* device, ID3D12CommandQueue* commandQueue) noexcept
    : m_device(device)
    , m_commandQueue(commandQueue)

    , m_fence(new D3DFence(device, 1U))
{
    LOG_DEBUG("D3DCommandListManager::D3DCommandListManager constructor %llx", this);
    ASSERT(m_commandQueue, "nullptr");
}

D3DCommandListManager::~D3DCommandListManager()
{
    LOG_DEBUG("D3DCommandListManager~D3DCommandListManager destructor %llx", this);
    if (m_commandQueue)
    {
        m_commandQueue->Release();
        m_commandQueue = nullptr;
    }

    D3DCommandListManager::wait();
    if (m_fence)
    {
        delete m_fence;
        m_fence = nullptr;
    }
}

D3DGraphicsCommandList* D3DCommandListManager::acquireCommandList(D3DCommandList::Type type)
{
    if (!m_freeCommandList[type].empty())
    {
        D3DCommandList* commandList = m_freeCommandList[type].front();
        m_freeCommandList[type].pop();

        return static_cast<D3DGraphicsCommandList*>(commandList);
    }

    ID3D12CommandAllocator* d3dCommandAllocator = nullptr;
    {
        HRESULT result = m_device->CreateCommandAllocator(D3DCommandList::convertCommandListTypeToD3DType(type), IID_PPV_ARGS(&d3dCommandAllocator));
        if (FAILED(result))
        {
            LOG_ERROR("D3DCommandListManager::acquireCommandList CreateCommandAllocator is failed. Error %s", D3DDebug::stringError(result).c_str());
            return nullptr;
        }
    }

    ID3D12CommandList* d3dCommandList = nullptr;
    {
        HRESULT result = m_device->CreateCommandList(0, D3DCommandList::convertCommandListTypeToD3DType(type), d3dCommandAllocator, nullptr, IID_PPV_ARGS(&d3dCommandList));
        if (FAILED(result))
        {
            LOG_ERROR("D3DCommandListManager::acquireCommandList CreateCommandList is failed. Error %s", D3DDebug::stringError(result).c_str());

            d3dCommandAllocator->Release();
            return nullptr;
        }
    }

    D3DGraphicsCommandList* commandList = new D3DGraphicsCommandList(m_device, type);
    ID3D12GraphicsCommandList* d3dGraphicCommandList = static_cast<ID3D12GraphicsCommandList*>(d3dCommandList);
    {
        HRESULT result = d3dGraphicCommandList->Close();
        if (FAILED(result))
        {
            LOG_ERROR("D3DCommandListManager::acquireCommandList Close commandList is failed. Error %s", D3DDebug::stringError(result).c_str());

            d3dCommandAllocator->Release();
            return nullptr;
        }
    }

    commandList->init(d3dGraphicCommandList, d3dCommandAllocator, true);
    commandList->m_status = D3DCommandList::Status::Initial;

    return commandList;
}

void D3DCommandListManager::clear()
{
    D3DCommandListManager::wait();
    D3DCommandListManager::update();

    ASSERT(m_usedCommandList.empty(), "must be empty");

    for (u32 i = 0; i < D3DCommandList::Type::CountTypes; ++i)
    {
        while (!m_freeCommandList[i].empty())
        {
            D3DCommandList* list = m_freeCommandList->front();
            m_freeCommandList->pop();

            list->destroy();
            delete list;
        }
    }
}

void D3DCommandListManager::update()
{
    for (std::list<D3DCommandList*>::iterator iter = m_usedCommandList.begin(); iter != m_usedCommandList.end();)
    {
        D3DCommandList* list = *iter;

        u64 value = list->m_fenceValue.load();
        u64 f = m_fence->getHandle()->GetCompletedValue();
        if (m_fence->completed(value))
        {
            LOG_WARNING("D3DCommandListManager::execute completed fence %u req %u", f, value);
            ASSERT(list->m_status == D3DCommandList::Status::Execute, "wrong status");
            list->m_status = D3DCommandList::Status::Finish;

            iter = m_usedCommandList.erase(iter);
            m_freeCommandList[list->m_type].push(list);

            continue;
        }
        else
        {
            LOG_WARNING("D3DCommandListManager::execute not completed fence %u req %u", f, value);
            int test = 0;
        }
        ++iter;
    }
}

bool D3DCommandListManager::execute(D3DCommandList* cmdList, bool wait)
{
    ASSERT(cmdList->m_status == D3DCommandList::Status::Closed, "wrong status");
    cmdList->m_status = D3DCommandList::Status::Execute;

    m_usedCommandList.push_back(cmdList);

    ID3D12CommandList* dxCommandLists[] = { cmdList->getHandle() };
    m_commandQueue->ExecuteCommandLists(1, dxCommandLists);

    ID3D12Fence* fence = m_fence->getHandle();

    u64 value = fence->GetCompletedValue();
    if (value == UINT64_MAX)
    {
        LOG_WARNING("D3DCommandListManager::execute device has been removed");
        D3DCommandListManager::wait();

        return false;
    }

    cmdList->m_fenceValue.store(value);
    ++value;

    HRESULT result = m_commandQueue->Signal(m_fence->getHandle(), value);
    if (FAILED(result))
    {
        LOG_ERROR("D3DCommandListManager::execute QueueSignal is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    if (wait)
    {
        if (!m_fence->wait(value))
        {
            return false;
        }
    }

    LOG_WARNING("D3DCommandListManager::execute  fence %u, value %u", fence->GetCompletedValue(), cmdList->m_fenceValue.load());

    return true;
}

bool D3DCommandListManager::wait()
{
    for (auto& list : m_usedCommandList)
    {
        u64 value = list->m_fenceValue.load();
        if (m_fence->wait(value))
        {
            LOG_ERROR("D3DCommandListManager::wait is failed");
            return false;
        }
    }

    return true;
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
