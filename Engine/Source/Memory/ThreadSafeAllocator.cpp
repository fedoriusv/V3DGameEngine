#include "ThreadSafeAllocator.h"

namespace v3d
{
namespace memory
{

ThreadSafeAllocator::ThreadSafeAllocator(u64 size, u32 workers)
    : m_poolSize(size)
{
    m_workerPools.resize(workers);
    for (auto& pool : m_workerPools)
    {
        pool._memory = V3D_MALLOC(m_poolSize, memory::MemoryLabel::MemoryDynamic);
        pool._offest = 0;
    }
}

ThreadSafeAllocator::~ThreadSafeAllocator()
{
    for (auto& pool : m_workerPools)
    {
        V3D_FREE(pool._memory, memory::MemoryLabel::MemoryDynamic);
    }
    m_workerPools.clear();
}

ThreadSafeAllocator::Allocation ThreadSafeAllocator::allocate(u64 size, u64 alignment, u32 workerID)
{
    ASSERT(alignment + size <= m_poolSize, "ThreadSafeAllocator overflow");
    u64 alignedSize = math::alignUp(size, alignment);

    auto& pool = m_workerPools[workerID];
    u64 currentOffest = pool._offest.fetch_add(alignedSize, std::memory_order_relaxed);

    return { static_cast<u8*>(pool._memory) + currentOffest, size };
}

void ThreadSafeAllocator::reset(u32 workerID)
{
    auto& pool = m_workerPools[workerID];
    pool._offest.store(0, std::memory_order_relaxed);
}

} //namespace memory
} //namespace v3d