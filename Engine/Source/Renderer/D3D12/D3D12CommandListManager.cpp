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

D3DCommandListManager::D3DCommandListManager(ID3D12Device* device, ID3D12CommandQueue* commandQueue, u32 swapImageCount) noexcept
    : m_device(device)
    , m_commandQueue(commandQueue)

    , m_currentIndex(-1)
{
    LOG_DEBUG("D3DCommandListManager::D3DCommandListManager constructor %llx", this);
    ASSERT(m_commandQueue, "nullptr");

    m_usedCommandList.resize(swapImageCount);
}

D3DCommandListManager::~D3DCommandListManager()
{
    LOG_DEBUG("D3DCommandListManager~D3DCommandListManager destructor %llx", this);
    ASSERT(m_usedCommandList.empty(), "must be empty");
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
            return nullptr;
        }
    }

    commandList->init(d3dGraphicCommandList, d3dCommandAllocator, true);
    commandList->m_status = D3DCommandList::Status::Initial;

    return commandList;
}

void D3DCommandListManager::waitAndClear()
{
    D3DCommandListManager::wait(true);
    D3DCommandListManager::update(true);
    for (u32 i = 0; i < m_usedCommandList.size(); ++i)
    {
        ASSERT(m_usedCommandList[i].empty(), "must be empty");
    }
    m_usedCommandList.clear();

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

bool D3DCommandListManager::update(bool updateAll)
{
    if (updateAll)
    {
        for (u32 i = 0; i < m_usedCommandList.size(); ++i)
        {
            for (std::list<D3DCommandList*>::iterator iter = m_usedCommandList[i].begin(); iter != m_usedCommandList[i].end();)
            {
                D3DCommandList* list = *iter;

                if (list->m_fence->completed())
                {
                    ASSERT(list->m_status == D3DCommandList::Status::Execute, "wrong status");
                    list->m_status = D3DCommandList::Status::Finish;

                    iter = m_usedCommandList[i].erase(iter);
                    m_freeCommandList[list->m_type].push(list);

                    continue;
                }
                ++iter;
            }

        }
    }
    else
    {
        for (std::list<D3DCommandList*>::iterator iter = m_usedCommandList[m_currentIndex].begin(); iter != m_usedCommandList[m_currentIndex].end();)
        {
            D3DCommandList* list = *iter;

            if (list->m_fence->completed())
            {
                ASSERT(list->m_status == D3DCommandList::Status::Execute, "wrong status");
                list->m_status = D3DCommandList::Status::Finish;

                iter = m_usedCommandList[m_currentIndex].erase(iter);
                m_freeCommandList[list->m_type].push(list);

                continue;
            }
            ++iter;
        }
    }

    return true;
}

bool D3DCommandListManager::execute(D3DCommandList* cmdList, bool wait)
{
    ASSERT(cmdList->m_status == D3DCommandList::Status::Closed, "wrong status");
    cmdList->m_status = D3DCommandList::Status::Execute;

    m_usedCommandList[m_currentIndex].push_back(cmdList);

    ID3D12CommandList* dxCommandLists[] = { cmdList->getHandle() };
    m_commandQueue->ExecuteCommandLists( 1, dxCommandLists );

    if (!cmdList->m_fence->signal(m_commandQueue))
    {
        LOG_ERROR("D3DCommandListManager::execute Fence signal is failed");
        return false;
    }

    return true;
}

bool D3DCommandListManager::wait(bool waitAll)
{
    if (waitAll)
    {
        for (u32 i = 0; i < m_usedCommandList.size(); ++i)
        {
            for (auto& list : m_usedCommandList[i])
            {
                if (!list->m_fence->wait())
                {
                    LOG_ERROR("D3DCommandListManager::wait is failed");
                    return false;
                }
            }
        }
    }
    else
    {
        for (auto& list : m_usedCommandList[m_currentIndex])
        {
            if (!list->m_fence->wait())
            {
                LOG_ERROR("D3DCommandListManager::wait is failed");
                return false;
            }
        }
    }

    return true;
}

bool D3DCommandListManager::sync(u32 index, bool wait)
{
    m_currentIndex = index;

    if (wait)
    {
        for (auto& list : m_usedCommandList[m_currentIndex])
        {
            if (!list->m_fence->wait())
            {
                LOG_ERROR("D3DCommandListManager::sync is failed");
                return false;
            }

            if (!list->m_fence->completed())
            {
                int error = 0;
            }
        }
    }

    return true;
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
