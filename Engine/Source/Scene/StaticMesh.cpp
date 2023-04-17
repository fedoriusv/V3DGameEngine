#include "StaticMesh.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

MeshHeader::MeshHeader() noexcept
    : ResourceHeader(resource::ResourceType::MeshResource)
    , _numVertices(0)
    , _vertexStride(0)
    , _numIndices(0)
    , _indexType(renderer::StreamIndexBufferType::IndexType_32)

    , _polygonMode(renderer::PolygonMode::PolygonMode_Fill)
    , _frontFace(renderer::FrontFace::FrontFace_Clockwise)
    , _vertexContentFlags(VertexProperies::VertexProperies_Empty)
    , _geometryContentFlags(GeometryContentFlag::Empty)
{
    static_assert(sizeof(MeshHeader) == sizeof(ResourceHeader) + 24, "wrong size");
}

u32 MeshHeader::operator>>(stream::Stream* stream) const
{
    u32 parentSize = ResourceHeader::operator>>(stream);
    u32 writePos = stream->tell();

    stream->write<u32>(_numVertices);
    stream->write<u32>(_numIndices);
    stream->write<u16>(_vertexStride);
    stream->write<renderer::StreamIndexBufferType>(_indexType);

    stream->write<renderer::PolygonMode>(_polygonMode);
    stream->write<renderer::FrontFace>(_frontFace);
    stream->write<VertexProperiesFlags>(_vertexContentFlags);
    stream->write<GeometryContentFlags>(_geometryContentFlags);

    u32 writeSize = stream->tell() - writePos + parentSize;
    ASSERT(sizeof(MeshHeader) == writeSize, "wrong size");
    return writeSize;
}

u32 MeshHeader::operator<<(const stream::Stream* stream)
{
    u32 parentSize = ResourceHeader::operator<<(stream);
    u32 readPos = stream->tell();

    stream->read<u32>(_numVertices);
    stream->read<u32>(_numIndices);
    stream->read<u16>(_vertexStride);
    stream->read<renderer::StreamIndexBufferType>(_indexType);

    stream->read<renderer::PolygonMode>(_polygonMode);
    stream->read<renderer::FrontFace>(_frontFace);
    stream->read<VertexProperiesFlags>(_vertexContentFlags);
    stream->read<GeometryContentFlags>(_geometryContentFlags);

    u32 readSize = stream->tell() - readPos + parentSize;
    ASSERT(sizeof(MeshHeader) == readSize, "wrong size");
    return readSize;
}

StaticMesh::StaticMesh() noexcept
    : m_header(nullptr)
    , m_indexBuffer({ nullptr, nullptr })
    , m_indexCount(0)
    , m_indexType(renderer::StreamIndexBufferType::IndexType_32)
    , m_vertexCount(0)
{
    LOG_DEBUG("StaticMesh constructor %llx", this);
}

StaticMesh::StaticMesh(MeshHeader* header) noexcept
    : m_header(header)
    , m_indexBuffer({ nullptr, nullptr})
    , m_indexCount(0)
    , m_indexType(renderer::StreamIndexBufferType::IndexType_32)
    , m_vertexCount(0)
{
    LOG_DEBUG("StaticMesh constructor %llx", this);
}

StaticMesh::~StaticMesh()
{
    LOG_DEBUG("StaticMesh destructor %llx", this);

    if (m_header->_geometryContentFlags & MeshHeader::GeometryContentFlag::IndexBuffer)
    {
        ASSERT(std::get<0>(m_indexBuffer), "nullptr");
        std::get<0>(m_indexBuffer)->unmap();
        stream::StreamManager::destroyStream(std::get<0>(m_indexBuffer));
        m_indexBuffer = { nullptr, nullptr };
        m_indexCount = 0;
    }

    for (auto& stream : m_vertexBuffers)
    {
        ASSERT(std::get<0>(stream), "nullptr");
        std::get<0>(stream)->unmap();
        stream::StreamManager::destroyStream(std::get<0>(stream));
    }
    m_vertexBuffers.clear();
    m_vertexCount = 0;

    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryObject);
        m_header = nullptr;
    }
}

bool StaticMesh::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("MeshStatic::load: the mesh %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);

    if (!m_header)
    {
        m_header = V3D_NEW(MeshHeader, memory::MemoryLabel::MemoryObject);
        ASSERT(m_header, "nullptr");
        m_header->operator<<(stream);
    }
    stream->seekBeg(m_header->_offset);

    m_description << stream;

    m_vertexCount = m_header->_numVertices;
    if (m_header->_geometryContentFlags & MeshHeader::GeometryContentFlag::SeparatePostion)
    {
        u32 positionBufferSize;
        stream->read<u32>(positionBufferSize);
        ASSERT(positionBufferSize == m_header->_numVertices * sizeof(math::Vector3D), "Should be same");

        stream::Stream* vertexBuffer = stream::StreamManager::createMemoryStream(nullptr, positionBufferSize);
        u8* vetexData = vertexBuffer->map(positionBufferSize);
        stream->read(vetexData, positionBufferSize);

        m_vertexBuffers.emplace_back(vertexBuffer, vetexData);
    }
    else
    {
        u32 vertexBufferSize;
        stream->read<u32>(vertexBufferSize);
        ASSERT(vertexBufferSize == m_header->_numVertices * m_header->_vertexStride, "Should be same");

        stream::Stream* vertexBuffer = stream::StreamManager::createMemoryStream(nullptr, vertexBufferSize);
        u8* vetexData = vertexBuffer->map(vertexBufferSize);
        stream->read(vetexData, vertexBufferSize);

        m_vertexBuffers.emplace_back(vertexBuffer, vetexData);
    }

    m_indexCount = m_header->_numIndices;
    if (m_header->_geometryContentFlags & MeshHeader::GeometryContentFlag::IndexBuffer)
    {
        u32 indexBufferSize;
        stream->read<u32>(indexBufferSize);

        u32 indexTypeSize = m_header->_indexType == renderer::StreamIndexBufferType::IndexType_16 ? sizeof(u16) : sizeof(u32);
        ASSERT(indexBufferSize == m_header->_numIndices * indexTypeSize, "Should be same");

        auto& [indexBuffer, indexData] = m_indexBuffer;
        indexBuffer = stream::StreamManager::createMemoryStream(nullptr, indexBufferSize);
        indexData = indexBuffer->map(indexBufferSize);
        stream->read(indexData, indexBufferSize);
    }

    if (m_header->_geometryContentFlags & MeshHeader::GeometryContentFlag::BoundingBox)
    {
        stream->read<math::AABB>(m_boundingBox);
    }

    LOG_DEBUG("StaticMesh::load: The stream has been read %d from %d bytes", stream->tell() - m_header->_offset, m_header->_size);

    m_loaded = true;
    return true;
}

bool StaticMesh::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

u64 StaticMesh::getVertexSize(u32 stream) const
{
    ASSERT(stream < m_vertexBuffers.size(), "range out");
    if (std::get<0>(m_vertexBuffers[stream]))
    {
        return std::get<0>(m_vertexBuffers[stream])->size();
    }

    return 0;
}

const void* StaticMesh::getVertexData(u32 stream) const
{
    ASSERT(stream < m_vertexBuffers.size(), "range out");
    if (std::get<1>(m_vertexBuffers[stream]))
    {
        return std::get<1>(m_vertexBuffers[stream]);
    }

    return nullptr;
}

u64 StaticMesh::getIndexSize() const
{
    if (std::get<0>(m_indexBuffer))
    {
        return std::get<0>(m_indexBuffer)->size();
    }

    return 0;
}

const void* StaticMesh::getIndexData() const
{
    if (std::get<1>(m_indexBuffer))
    {
        return std::get<1>(m_indexBuffer);
    }

    return nullptr;
}

} //namespace scene
} //namespace v3d