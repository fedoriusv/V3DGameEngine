#include "StreamManager.h"
#include "MemoryStream.h"
#include "Utils/MemoryPool.h"

namespace v3d
{
namespace stream
{

    utils::MemoryPool* StreamManager::m_memoryPool = new utils::MemoryPool(4096, 1024, 4096 * 2, utils::MemoryPool::getDefaultMemoryPoolAllocator());

MemoryStream* StreamManager::createMemoryStream(const void* data, const u32 size)
{
    return new MemoryStream(data, size, m_memoryPool);
}

} //namespace stream
} //namespace v3d
