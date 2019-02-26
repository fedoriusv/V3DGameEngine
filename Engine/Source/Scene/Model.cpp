#include "Model.h"
#include "Stream/StreamManager.h"

namespace v3d
{
namespace scene
{

Model::Model(ModelHeader* header) noexcept
    : Resource(header)
    , m_vertexModelData(nullptr)
    , m_indexModelData(nullptr)
{
}

Model::~Model()
{
    for (auto& mesh : m_meshes)
    {
        delete mesh;
    }
    m_meshes.clear();

    if (m_vertexModelData)
    {
        delete m_vertexModelData;
        m_vertexModelData = nullptr;
    }

    if (m_indexModelData)
    {
        delete m_indexModelData;
        m_indexModelData = nullptr;
    }
}

const ModelHeader & Model::getModelHeader() const
{
    return *(static_cast<const scene::ModelHeader*>(m_header));
}

Model::Mesh * Model::getMeshByIndex(u32 index) const
{
    ASSERT(index < m_meshes.size(), "range out");
    return m_meshes[index];
}


void Model::init(stream::Stream * stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Model::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");
    m_stream->seekBeg(0);

    const ModelHeader& header = Model::getModelHeader();

    m_vertexModelData = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(header._vertex._globalSize));
    u8* vetexData = m_vertexModelData->map(static_cast<u32>(header._vertex._globalSize));
    m_stream->read(vetexData, static_cast<u32>(header._vertex._globalSize), 1);

    m_meshes.resize(header._vertex._countElements);

    u8* indexData = nullptr;
    if (header._index._present)
    {
        m_indexModelData = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(header._index._globalSize));
        indexData = m_indexModelData->map(static_cast<u32>(header._index._globalSize));
        m_stream->read(indexData, static_cast<u32>(header._index._globalSize), 1);
        m_indexModelData->unmap();
    }

    for (u32 meshIndex = 0; meshIndex < m_meshes.size(); ++meshIndex)
    {
        Mesh* mesh = new Mesh();

        u8* vertexDataPtr = vetexData + header._vertex._data[meshIndex]._offset;
        u32 vertexSize = static_cast<u32>(header._vertex._data[meshIndex]._size);
        mesh->fillVertexData(static_cast<u32>(header._vertex._data[meshIndex]._count), vertexDataPtr, vertexSize);

        if (header._index._present)
        {
            u8* indexDataPtr = indexData + header._index._data[meshIndex]._offset;
            u32 indexSize = static_cast<u32>(header._index._data[meshIndex]._size);
            mesh->fillIndexData(static_cast<u32>(header._index._data[meshIndex]._count), indexDataPtr, indexSize);
        }

        m_meshes[meshIndex] = mesh;
    }
    m_vertexModelData->unmap();



    for (u32 vertBuffIndex = 0; vertBuffIndex < header._vertex._countElements; ++vertBuffIndex)
    {
        m_meshes[vertBuffIndex]->m_description << m_stream;
    }

    ASSERT(!m_stream->isMapped(), "mapped");
    delete m_stream;
    m_stream = nullptr;

    m_loaded = true;
    return true;
}

ModelHeader::ModelHeader() noexcept
    : _mode(renderer::PolygonMode::PolygonMode_Triangle)
    , _frontFace(renderer::FrontFace::FrontFace_Clockwise)
    , _modelContent(ModelContent::ModelContext_Empty)
    , _vertexContent(VertexProperies::VertexProperies_Empty)
    , _localTransform(false)
{
    memset(&_vertex, 0, sizeof(MeshInfo));
    memset(&_index, 0, sizeof(MeshInfo));
    memset(&_materials, 0, sizeof(MaterialInfo));
}

ModelHeader::~ModelHeader()
{
}

Model::Mesh::Mesh() noexcept
    : m_vertexCount(0)
    , m_indexCount(0)
{
    memset(&m_vertexData, 0, sizeof(BufferData));
    memset(&m_indexData, 0, sizeof(BufferData));
}

Model::Mesh::~Mesh()
{
}

u8 * Model::Mesh::getVertexData() const
{
    ASSERT(m_vertexData._data, "nullptr");
    return m_vertexData._data;
}

u8 * Model::Mesh::getIndexData() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._data;
}

const renderer::VertexInputAttribDescription & Model::Mesh::getVertexInputAttribDesc() const
{
    return m_description;
}

void Model::Mesh::fillVertexData(u32 count, u8 * data, u32 size)
{
    ASSERT(!m_vertexData._data, "not nullptr");
    m_vertexCount = count;
    m_vertexData._data = data;
    m_vertexData._size = size;
}


void Model::Mesh::fillIndexData(u32 count, u8 * data, u32 size)
{
    ASSERT(!m_indexData._data, "not nullptr");
    m_indexCount = count;
    m_indexData._data = data;
    m_indexData._size = size;
}

} //namespace scene
} //namespace v3d
