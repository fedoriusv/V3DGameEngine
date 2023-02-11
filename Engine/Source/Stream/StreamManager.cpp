#include "StreamManager.h"
#include "MemoryStream.h"
#include "Utils/MemoryPool.h"

namespace v3d
{
namespace stream
{

utils::MemoryPool* StreamManager::s_memoryPool = nullptr;

MemoryStream* StreamManager::createMemoryStream(const void* data, const u32 size)
{
    if (!s_memoryPool)
    {
        s_memoryPool = new utils::MemoryPool(64 * 1024, utils::MemoryPool::getDefaultMemoryAllocator());
    }

    return new MemoryStream(data, size, s_memoryPool);
}

const MemoryStream* StreamManager::createMemoryStream(const std::string& string)
{
    MemoryStream* memory = StreamManager::createMemoryStream(nullptr, static_cast<u32>(string.length()));
    memory->write(string.data(), static_cast<u32>(string.length()));

    return memory;
}

void StreamManager::clearPools()
{
    s_memoryPool->clear();
}

} //namespace stream
} //namespace v3d
