#include "Mesh.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

MeshHeader::MeshHeader() noexcept
    : _numVertices(0)
    , _vertexStride(0)
    , _numIndices(0)
    , _indexType(renderer::StreamIndexBufferType::IndexType_32)

    , _polygonMode(renderer::PolygonMode::PolygonMode_Fill)
    , _frontFace(renderer::FrontFace::FrontFace_Clockwise)
    , _vertexContentFlags(VertexProperies::VertexProperies_Empty)
    , _geometryContentFlags(GeometryContentFlag::Empty)
{
}

u32 MeshHeader::operator>>(stream::Stream* stream)
{
    u32 write = ResourceHeader::operator>>(stream);

    write += stream->write<u32>(_numVertices);
    write += stream->write<u32>(_vertexStride);
    write += stream->write<u32>(_numIndices);
    write += stream->write<renderer::StreamIndexBufferType>(_indexType);

    write += stream->write<renderer::PolygonMode>(_polygonMode);
    write += stream->write<renderer::FrontFace>(_frontFace);
    write += stream->write<VertexProperiesFlags>(_vertexContentFlags);
    write += stream->write<GeometryContentFlags>(_geometryContentFlags);

    return write;
}

u32 MeshHeader::operator<<(const stream::Stream* stream)
{
    u32 read = ResourceHeader::operator<<(stream);

    read += stream->read<u32>(_numVertices);
    read += stream->read<u32>(_vertexStride);
    read += stream->read<u32>(_numIndices);
    read += stream->read<renderer::StreamIndexBufferType>(_indexType);

    read += stream->read<renderer::PolygonMode>(_polygonMode);
    read += stream->read<renderer::FrontFace>(_frontFace);
    read += stream->read<VertexProperiesFlags>(_vertexContentFlags);
    read += stream->read<GeometryContentFlags>(_geometryContentFlags);

    return read;
}

Mesh::Mesh(const MeshHeader* header) noexcept
    : Resource(header)
    , m_indexBuffer({ nullptr, nullptr})
    , m_indexCount(0)
    , m_indexType(renderer::StreamIndexBufferType::IndexType_32)
    , m_vertexCount(0)
{
    LOG_DEBUG("Mesh constructor %xll", this);
}

Mesh::~Mesh()
{
    LOG_DEBUG("Model destructor %xll", this);
    const MeshHeader& header = Mesh::getMeshHeader();

    //if (header._flags & MeshHeader::GeometryFlag::IndexBuffer)
    //{
    //    //m_indexBuffer->unmap();
    //    //stream::StreamManager::destroyStream(m_indexBuffer);
    //    //m_indexBuffer = nullptr;
    //    //m_indexData = nullptr;
    //}


}

void Mesh::init(stream::Stream* stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}
bool Mesh::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");

    //const MeshHeader& header = Mesh::getMeshHeader();
    //m_stream->seekBeg(header._offset);

    //m_description << m_stream;

    //m_vertexCount = header._vertices;
    //if (header._flags & MeshHeader::GeometryFlag::SeparatePostionAttribute)
    //{
    //    u32 positionBufferSize = header._vertices * header._stride;
    //    ASSERT(positionBufferSize, "Position buffer is empty");

    //    stream::Stream* vertexBuffer = stream::StreamManager::createMemoryStream(nullptr, positionBufferSize);
    //    u8* vetexData = vertexBuffer->map(header._size);
    //    m_stream->read(vetexData, header._size, 1);
    //    vertexBuffer->unmap();

    //    m_vertexBuffers.emplace_back(vertexBuffer, vetexData);
    //}
    //else
    //{
    //    u32 vertexBufferSize = header._vertices * header._stride;
    //    ASSERT(vertexBufferSize, "Vertex buffer is empty");

    //    stream::Stream* vertexBuffer = stream::StreamManager::createMemoryStream(nullptr, vertexBufferSize);
    //    u8* vetexData = vertexBuffer->map(header._size);
    //    m_stream->read(vetexData, header._size, 1);
    //    vertexBuffer->unmap();

    //    m_vertexBuffers.emplace_back(vertexBuffer, vetexData);
    //}

    //if (header._flags & MeshHeader::GeometryFlag::IndexBuffer)
    //{
    //    m_indexCount = header._indices;
    //    m_indexType = header._indexType == 0 ? renderer::StreamIndexBufferType::IndexType_16 : renderer::StreamIndexBufferType::IndexType_32;

    //    u32 indexBufferSize = header._indices * (header._indexType == 0 ? sizeof(u16) : sizeof(u32));
    //    ASSERT(indexBufferSize, "Index buffer is empty");

    //    auto& [indexBuffer, indexData] = m_indexBuffer;
    //    indexBuffer = stream::StreamManager::createMemoryStream(nullptr, indexBufferSize);
    //    indexData = indexBuffer->map(header._size);
    //    m_stream->read(indexData, header._size, 1);
    //}

    //if (header._flags & MeshHeader::GeometryFlag::BoundingBox)
    //{
    //    m_boundingBox << m_stream;
    //}

    //ASSERT(!m_stream->isMapped(), "mapped");
    //delete m_stream;
    //m_stream = nullptr;

    m_loaded = true;

    return true;
}

const renderer::VertexInputAttributeDescription& Mesh::getInputAttributeDesc() const
{
    return m_description;
}

u32 Mesh::getVertexCount() const
{
    return m_vertexCount;
}

u64 Mesh::getVertexSize(u32 stream) const
{
    ASSERT(stream < m_vertexBuffers.size(), "range out");
    if (std::get<0>(m_vertexBuffers[stream]))
    {
        std::get<0>(m_vertexBuffers[stream])->size();
    }

    return 0;
}

const u8* Mesh::getVertexData(u32 stream) const
{
    ASSERT(stream < m_vertexBuffers.size(), "range out");
    if (std::get<1>(m_vertexBuffers[stream]))
    {
        return std::get<1>(m_vertexBuffers[stream]);
    }

    return nullptr;
}

u32 Mesh::getIndexCount() const
{
    return m_indexCount;
}

u64 Mesh::getIndexSize() const
{
    if (std::get<0>(m_indexBuffer))
    {
        return std::get<0>(m_indexBuffer)->size();
    }

    return 0;
}

const u8* Mesh::getIndexData() const
{
    if (std::get<1>(m_indexBuffer))
    {
        return std::get<1>(m_indexBuffer);
    }

    return nullptr;
}

const MeshHeader& Mesh::getMeshHeader() const
{
    ASSERT(m_header, "nullptr");
    return *static_cast<const MeshHeader*>(m_header);
}

} //namespace scene
} //namespace v3d