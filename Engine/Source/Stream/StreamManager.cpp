#include "StreamManager.h"
#include "MemoryStream.h"
#include "Utils/MemoryPool.h"

namespace v3d
{
namespace stream
{

const u32 k_initialPoolSize = 1024 * 1024 * 8;
utils::MemoryPool* StreamManager::m_memoryPool = new utils::MemoryPool(k_initialPoolSize, 64, 1024 * 1024 * 8, utils::MemoryPool::getDefaultMemoryPoolAllocator());

MemoryStream* StreamManager::createMemoryStream(const void* data, const u32 size)
{
    return new MemoryStream(data, size, nullptr);//m_memoryPool);
}

MemoryStream* StreamManager::createMemoryStream(const std::string& string)
{
    MemoryStream* memory = StreamManager::createMemoryStream(nullptr, static_cast<u32>(string.length()) + sizeof(u32));
    memory->write(static_cast<u32>(string.length()));
    memory->write(string.data(), static_cast<u32>(string.length()));

    return memory;
}

void StreamManager::clearPools()
{
    m_memoryPool->clearPools();
}

} //namespace stream
} //namespace v3d
