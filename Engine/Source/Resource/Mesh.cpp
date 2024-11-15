#include "Mesh.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{

MeshResource::MeshResource() noexcept
    : m_header()
{
}

MeshResource::MeshResource(const MeshHeader& header) noexcept
    : m_header(header)
{
}

MeshResource::~MeshResource()
{
}

bool MeshResource::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("MeshResource::load: the mesh %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "stream is nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    m_description << stream;

    u32 streamsCount;
    stream->read<u32>(streamsCount);
    for (u32 streamNo = 0; streamNo < streamsCount; ++streamNo)
    {
        stream->read<u32>(m_vertexData._vertexCount);

        u32 streamSize;
        stream->read<u32>(streamSize);
        stream::Stream* vertexData = stream::StreamManager::createMemoryStream(nullptr, streamSize);
        void* ptr = vertexData->map(vertexData->size());
        stream->read(ptr, vertexData->size());

        m_vertexData._vertexBuffers.push_back(std::make_tuple(vertexData, ptr));
    }

    stream->read<u32>(m_indexData._indexCount);
    if (m_indexData._indexCount > 0)
    {
        bool isIndexType32;
        stream->read<bool>(isIndexType32);
        m_indexData._indexType = isIndexType32 ? renderer::IndexBufferType::IndexType_32 : renderer::IndexBufferType::IndexType_16;

        u32 streamSize = m_indexData._indexCount * (isIndexType32 ? sizeof(u32) : sizeof(u16));
        stream::Stream* indexData = stream::StreamManager::createMemoryStream(nullptr, streamSize);
        void* ptr = indexData->map(indexData->size());
        stream->read(ptr, indexData->size());

        m_indexData._indexBuffer = std::make_tuple(indexData, ptr);
    }

    return true;
}

bool MeshResource::save(stream::Stream* stream, u32 offset) const
{
    return false;
}

} //namespace resource
} //namespace v3d