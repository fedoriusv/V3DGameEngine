#include "StreamManager.h"
#include "MemoryStream.h"

namespace v3d
{
namespace stream
{

MemoryStream* StreamManager::createMemoryStream(const void* data, const u32 size)
{
    return ::V3D_NEW(MemoryStream, memory::MemoryLabel::MemorySystem)(data, size);
}

const MemoryStream* StreamManager::createMemoryStream(const std::string& string)
{
    MemoryStream* memory = StreamManager::createMemoryStream(nullptr, static_cast<u32>(string.length()));
    memory->write(string.data(), static_cast<u32>(string.length()));

    return memory;
}

void StreamManager::destroyStream(const Stream* stream)
{
    ASSERT(stream, "nullptr");
    V3D_DELETE(stream, memory::MemoryLabel::MemorySystem);
}

} //namespace stream
} //namespace v3d
