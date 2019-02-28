#include "Mesh.h"
#include "Stream/StreamManager.h"
#include "Scene/Model.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

MeshHeader::MeshHeader()
    : _size(0)
    , _offset(0)
{
}

MeshHeader::GeometryInfo::GeometryInfo()
    : _count(0)
    , _size(0)
{
}

void MeshHeader::GeometryInfo::operator >> (stream::Stream * stream) const
{
    stream->write<u32>(_count);
    stream->write<u64>(_size);

    u32 count = static_cast<u32>(_subData.size());
    stream->write<u32>(count);
    stream->write(_subData.data(), count * sizeof(SubData), 1);
}

void MeshHeader::GeometryInfo::operator << (const stream::Stream * stream)
{
    stream->read<u32>(_count);
    stream->read<u64>(_size);

    u32 count;
    stream->read<u32>(count);
    _subData.resize(count);
    stream->read(_subData.data(), count * sizeof(SubData), 1);
}

Mesh::SubMesh::SubMesh() noexcept
    : m_vertexCount(0)
    , m_indexCount(0)
{
    memset(&m_indexData, 0, sizeof(BufferData));
    memset(&m_vertexData, 0, sizeof(BufferData));
}

Mesh::SubMesh::~SubMesh()
{
}

u8 * Mesh::SubMesh::getVertexData() const
{
    ASSERT(m_vertexData._data, "nullptr");
    return m_vertexData._data;
}

u32 Mesh::SubMesh::getVertexCount() const
{
    return m_vertexCount;
}

u64 Mesh::SubMesh::getVertexSize() const
{
    ASSERT(m_vertexData._data, "nullptr");
    return m_vertexData._size;
}

u8 * Mesh::SubMesh::getIndexData() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._data;
}

u32 Mesh::SubMesh::getIndexCount() const
{
    return m_indexCount;
}

u64 Mesh::SubMesh::getIndexSize() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._size;
}

void Mesh::SubMesh::fillVertexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_vertexData._data, "not nullptr");
    m_vertexCount = count;
    m_vertexData._data = data;
    m_vertexData._size = size;
}


void Mesh::SubMesh::fillIndexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_indexData._data, "not nullptr");
    m_indexCount = count;
    m_indexData._data = data;
    m_indexData._size = size;
}

Mesh::Mesh(MeshHeader* header) noexcept
    : Resource(header)
    , m_indexStreamBuffer(nullptr)
    , m_vertexStreamBuffer(nullptr)

    , m_indexCount(0)
    , m_vertexCount(0)
{
    LOG_DEBUG("Mesh constructor %xll", this);

    memset(&m_indexData, 0, sizeof(BufferData));
    memset(&m_vertexData, 0, sizeof(BufferData));
}

Mesh::~Mesh()
{
    LOG_DEBUG("Mesh destructor %xll", this);

    for (auto& mesh : m_subMeshes)
    {
        delete mesh;
    }
    m_subMeshes.clear();

    if (m_vertexStreamBuffer)
    {
        delete m_vertexStreamBuffer;
        m_vertexStreamBuffer = nullptr;
    }

    if (m_indexStreamBuffer)
    {
        delete m_indexStreamBuffer;
        m_indexStreamBuffer = nullptr;
    }
}


void Mesh::init(stream::Stream * stream)
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
    m_stream->seekBeg(0);

    m_description << m_stream;

    const MeshHeader::GeometryInfo& vertexInfo = Mesh::getMeshHeader()._vertex;
    const MeshHeader::GeometryInfo& indexInfo = Mesh::getMeshHeader()._index;

    m_vertexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(vertexInfo._size));
    u8* vetexData = m_vertexStreamBuffer->map(static_cast<u32>(vertexInfo._size));
    m_stream->read(vetexData, static_cast<u32>(vertexInfo._size), 1);
    Mesh::fillVertexData(vertexInfo._count, vetexData, vertexInfo._size);

    m_subMeshes.resize(static_cast<u32>(vertexInfo._subData.size()));

    u8* indexData = nullptr;
    if (Mesh::getMeshHeader()._indexPresent)
    {
        m_indexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(indexInfo._size));
        indexData = m_indexStreamBuffer->map(static_cast<u32>(indexInfo._size));
        m_stream->read(indexData, static_cast<u32>(indexInfo._size), 1);
        m_indexStreamBuffer->unmap();

        Mesh::fillIndexData(indexInfo._count, indexData, indexInfo._size);
    }

    for (u32 meshIndex = 0; meshIndex < m_subMeshes.size(); ++meshIndex)
    {
        SubMesh* mesh = new SubMesh();

        u8* vertexDataPtr = vetexData + vertexInfo._subData[meshIndex]._offset;
        u64 vertexSize = vertexInfo._subData[meshIndex]._size;
        mesh->fillVertexData(vertexInfo._subData[meshIndex]._count, vertexDataPtr, vertexSize);

        if (Mesh::getMeshHeader()._indexPresent)
        {
            u8* indexDataPtr = indexData + indexInfo._subData[meshIndex]._offset;
            u64 indexSize = indexInfo._subData[meshIndex]._size;
            mesh->fillIndexData(indexInfo._subData[meshIndex]._count, indexDataPtr, indexSize);
        }

        m_subMeshes[meshIndex] = mesh;
    }
    m_vertexStreamBuffer->unmap();

    ASSERT(!m_stream->isMapped(), "mapped");
    delete m_stream;
    m_stream = nullptr;

    m_loaded = true;

    return true;
}

const renderer::VertexInputAttribDescription & Mesh::getVertexInputAttribDesc() const
{
    return m_description;
}

u8 * Mesh::getVertexData(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        ASSERT(m_vertexData._data, "nullptr");
        return m_vertexData._data;
    }
    return m_subMeshes[subMesh]->getVertexData();
}

u32 Mesh::getVertexCount(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        return m_vertexCount;
    }
    return m_subMeshes[subMesh]->getVertexCount();
}

u64 Mesh::getVertexSize(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        ASSERT(m_vertexData._data, "nullptr");
        return m_vertexData._size;
    }
    return m_subMeshes[subMesh]->getVertexSize();
}

u8 * Mesh::getIndexData(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        ASSERT(m_indexData._data, "nullptr");
        return m_indexData._data;
    }
    return m_subMeshes[subMesh]->getIndexData();
}

u32 Mesh::getIndexCount(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        return m_indexCount;
    }
    return m_subMeshes[subMesh]->getVertexCount();
}

u64 Mesh::getIndexSize(s32 subMesh) const
{
    ASSERT(m_subMeshes.size() > subMesh || subMesh < 0, "out of range");
    if (subMesh < 0)
    {
        ASSERT(m_indexData._data, "nullptr");
        return m_indexData._size;
    }
    return m_subMeshes[subMesh]->getIndexSize();
}

const MeshHeader & Mesh::getMeshHeader() const
{
    return *static_cast<const MeshHeader*>(m_header);
}

void Mesh::fillVertexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_vertexData._data, "not nullptr");
    m_vertexCount = count;
    m_vertexData._data = data;
    m_vertexData._size = size;
}

void Mesh::fillIndexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_indexData._data, "not nullptr");
    m_indexCount = count;
    m_indexData._data = data;
    m_indexData._size = size;
}

} //namespace scene
} //namespace v3d
