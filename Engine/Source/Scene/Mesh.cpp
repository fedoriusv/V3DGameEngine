#include "Mesh.h"
#include "Stream/StreamManager.h"
#include "Scene/Model.h"

namespace v3d
{
namespace scene
{

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

u8 * Mesh::SubMesh::getIndexData() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._data;
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

Mesh::Mesh() noexcept
    : m_indexStreamBuffer(nullptr)
    , m_vertexStreamBuffer(nullptr)
{
}

Mesh::~Mesh()
{
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

    ModelHeader::GeometryInfo vertexInfo;
    vertexInfo << m_stream;
    ASSERT(vertexInfo._present, "vertex should be present");

    ModelHeader::GeometryInfo indexInfo;
    indexInfo << m_stream;

    m_vertexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(vertexInfo._size));
    u8* vetexData = m_vertexStreamBuffer->map(static_cast<u32>(vertexInfo._size));
    m_stream->read(vetexData, static_cast<u32>(vertexInfo._size), 1);

    m_subMeshes.reserve(static_cast<u32>(vertexInfo._subData.size()));

    u8* indexData = nullptr;
    if (indexInfo._present)
    {
        m_indexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(indexInfo._size));
        indexData = m_indexStreamBuffer->map(static_cast<u32>(indexInfo._size));
        m_stream->read(indexData, static_cast<u32>(indexInfo._size), 1);
        m_indexStreamBuffer->unmap();
    }

    for (u32 meshIndex = 0; meshIndex < m_subMeshes.size(); ++meshIndex)
    {
        SubMesh* mesh = new SubMesh();

        u8* vertexDataPtr = vetexData + vertexInfo._subData[meshIndex]._offset;
        u64 vertexSize = vertexInfo._subData[meshIndex]._size;
        mesh->fillVertexData(vertexInfo._subData[meshIndex]._count, vertexDataPtr, vertexSize);

        if (indexInfo._present)
        {
            u8* indexDataPtr = indexData + indexInfo._subData[meshIndex]._offset;
            u64 indexSize = indexInfo._subData[meshIndex]._size;
            mesh->fillIndexData(indexInfo._subData[meshIndex]._count, indexDataPtr, indexSize);
        }

        m_subMeshes[meshIndex] = mesh;
    }
    m_vertexStreamBuffer->unmap();

    m_description << m_stream;

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

} //namespace scene
} //namespace v3d
