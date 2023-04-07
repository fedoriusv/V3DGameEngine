#include "D3DQueryHeap.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DDeviceCaps.h"
#include "D3DCommandList.h"
#include "D3DBuffer.h"
namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3D12_QUERY_TYPE D3DRenderQuery::convertQueryTypeToD3D(QueryType type) noexcept
{
    switch (type)
    {
    case QueryType::Occlusion:
        return D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_OCCLUSION;

    case QueryType::BinaryOcclusion:
        return D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_BINARY_OCCLUSION;

    case QueryType::TimeStamp:
        return D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP;

    default:
        ASSERT(false, "not supported");
    }

    return D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_OCCLUSION;
}

D3DRenderQuery::D3DRenderQuery(ID3D12Device* device, QueryType type, u32 count, const QueryRespose& callback, const std::string& name, D3DHeapAllocator* allocator) noexcept
    : Query(type, count, callback)
{
}

D3DRenderQuery::~D3DRenderQuery()
{
}

bool D3DRenderQuery::create()
{
    return true;
}

void D3DRenderQuery::destroy()
{
}

void D3DRenderQuery::invoke(QueryResult result, u32 size, const void* data)
{
    if (m_callback)
    {
        ASSERT(size == m_count * sizeof(u64), "wrong size");
        std::invoke(m_callback, result, size, data);
    }
}


D3D12_QUERY_HEAP_TYPE D3DQueryHeap::convertQueryTypeToHeapType(D3D12_QUERY_TYPE type) noexcept
{
    switch (type)
    {
    case D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_OCCLUSION:
    case D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_BINARY_OCCLUSION:
        return D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_OCCLUSION;

    case D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP:
        return D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

    default:
        ASSERT(false, "unknown");

    }

    return D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_OCCLUSION;
}

D3DQueryHeap::D3DQueryHeap(ID3D12Device* device, QueryType type, u32 count, D3DHeapAllocator* allocator) noexcept
    : m_device(device)
    , m_allocator(allocator)

    , m_queryHeap(nullptr)
    , m_type(D3DRenderQuery::convertQueryTypeToD3D(type))
    , m_count(count)
    , m_resolveBuffer(nullptr)
{
    ASSERT(m_count <= D3DDeviceCaps::getInstance()->queryHeapCount, "must be less or heap is too small");
    LOG_DEBUG("D3DQueryHeap::D3DQueryHeap constructor %llx", this);
}

D3DQueryHeap::~D3DQueryHeap()
{
    LOG_DEBUG("D3DQueryHeap::D3DQueryHeap destructor %llx", this);
    ASSERT(!m_queryHeap, "nullptr");
}

bool D3DQueryHeap::create()
{
    D3D12_QUERY_HEAP_DESC desc{};
    desc.Type = D3DQueryHeap::convertQueryTypeToHeapType(m_type);
    desc.Count = m_count;
    desc.NodeMask = 0;

    HRESULT result = m_device->CreateQueryHeap(&desc, DX_IID_PPV_ARGS(&m_queryHeap));
    if (FAILED(result))
    {
        LOG_ERROR("D3DQueryHeap::create CreateQueryHeap is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    m_resolveBuffer = new D3DBuffer(m_device, Buffer::BufferType::Readback, StreamBufferUsage::StreamBuffer_Read, desc.Count * sizeof(u64), "QueryBuffer", m_allocator);
    if (!m_resolveBuffer->create())
    {
        LOG_ERROR("D3DQueryHeap::create can't create resolve buffer");
        destroy();

        return false;
    }


    return true;
}

void D3DQueryHeap::destroy()
{
    if (m_resolveBuffer)
    {
        m_resolveBuffer->destroy();
        delete m_resolveBuffer;
    }
    SAFE_DELETE(m_queryHeap);
}



D3DRenderQueryManager::D3DRenderQueryManager(ID3D12Device* device, D3DHeapAllocator* allocator) noexcept
    : m_device(device)
    , m_allocator(allocator)
{
    memset(m_currentHeap, 0, sizeof(m_currentHeap));
}

D3DRenderQueryManager::~D3DRenderQueryManager()
{
    for (u32 type = toEnumType(QueryType::First); type < toEnumType(QueryType::Count); ++type)
    {
        ASSERT(!m_currentHeap[type], "must be nullptr");
        ASSERT(m_usedHeaps[type].empty(), "must be empty");
        ASSERT(m_freeHeaps[type].empty(), "must be empty");
    }
    ASSERT(m_activeRenderQueries.empty(), "must be empty");
}

D3DQueryHeap* D3DRenderQueryManager::acquireQueryHeap(QueryType type)
{
    D3DQueryHeap* heap = nullptr;
    if (m_currentHeap[toEnumType(type)])
    {
        heap = m_currentHeap[toEnumType(type)];
    }
    else
    {
        if (m_freeHeaps[toEnumType(type)].empty())
        {
            heap = new D3DQueryHeap(m_device, type, D3DDeviceCaps::getInstance()->queryHeapCount, m_allocator);
            if (!heap->create())
            {
                LOG_ERROR("RenderQueryManager::acquireQueryHeap create is failed");

                delete heap;
                return nullptr;
            }
        }
        else
        {
            heap = m_freeHeaps[toEnumType(type)].front();
            m_freeHeaps[toEnumType(type)].pop();
        }

        m_currentHeap[toEnumType(type)] = heap;
    }

    return heap;
}

void D3DRenderQueryManager::resolve(D3DGraphicsCommandList* cmdList)
{
    ASSERT(cmdList->isReadyForRecord(), "must be open");
    for (auto& iter : m_activeRenderQueries)
    {
        D3DRenderQuery* query = iter.first;
        auto& [heap, range] = iter.second;

        ASSERT(heap->m_resolveBuffer->getState() == D3D12_RESOURCE_STATE_COPY_DEST, "must be copy state");
        cmdList->resolveQuery(heap, range.start, range.count, heap->m_resolveBuffer, 0);

        auto inserted = m_resolvedQueries.emplace(heap, std::make_tuple(query, range.count));
        ASSERT(inserted.second, "must be true");
    }

    m_activeRenderQueries.clear();
}

void D3DRenderQueryManager::update(bool forceFinish)
{
    for (u32 type = toEnumType(QueryType::First); type < toEnumType(QueryType::Count); ++type)
    {
        if (m_currentHeap[type])
        {
            m_usedHeaps[type].push_back(m_currentHeap[type]);
            m_currentHeap[type] = nullptr;
        }

        for (auto iter = m_usedHeaps[type].begin(); iter != m_usedHeaps[type].end();)
        {
            D3DQueryHeap* heap = (*iter);
            D3DBuffer* buffer = heap->m_resolveBuffer;
            if (!heap->isUsed() && !buffer->isUsed())
            {
                [[unlikely]] if (forceFinish)
                {
                    m_resolvedQueries.clear();
                }
                else
                {
                    auto found = m_resolvedQueries.find(heap);
                    ASSERT(found != m_resolvedQueries.end(), "must be found");

                    D3DRenderQuery* query = std::get<0>(found->second);

                    u32 size = std::get<1>(found->second);
                    const void* data = buffer->map(0, size * sizeof(u64), true);
                    query->invoke(QueryResult::Success, size * sizeof(u64), data);
                    buffer->unmap(0, size * sizeof(u64));

                    m_resolvedQueries.erase(found);
                }
                iter = m_usedHeaps[type].erase(iter);
                m_freeHeaps[type].push(heap);

                continue;
            }

            ++iter;
        }
    }
}

void D3DRenderQueryManager::clear()
{
    m_resolvedQueries.clear();
    for (u32 type = toEnumType(QueryType::First); type < toEnumType(QueryType::Count); ++type)
    {
        ASSERT(!m_currentHeap[type], "Must be nupllptr");
        ASSERT(m_usedHeaps[type].empty(), "Must be empty");

       while (!m_freeHeaps[type].empty())
       {
           D3DQueryHeap* heap = m_freeHeaps[type].front();
           m_freeHeaps[type].pop();

           ASSERT(!heap->isUsed() && !heap->m_resolveBuffer->isUsed(), "Must be free");
           heap->destroy();
           delete heap;
       }
    }
}

bool D3DRenderQueryManager::bind(D3DRenderQuery* query, D3DQueryHeap* heap, u32 start, u32 count)
{
    auto inserted = m_activeRenderQueries.emplace(query, std::make_tuple(heap, D3DRenderQueryManager::Range{ start, count }));
    if (inserted.second)
    {
        return true;
    }

    return false;
}

D3DQueryHeap* D3DRenderQueryManager::find(D3DRenderQuery* query) const
{
    auto found = m_activeRenderQueries.find(query);
    if (found != m_activeRenderQueries.end())
    {
        return std::get<0>(found->second);
    }

    return nullptr;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER