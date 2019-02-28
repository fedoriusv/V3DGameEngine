#include "Model.h"
#include "Stream/StreamManager.h"
#include "Scene/Mesh.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

ModelHeader::ModelHeader() noexcept
    : _modelContentFlags(ModelContent::ModelContext_Empty)
    , _vertexContentFlags(VertexProperies::VertexProperies_Empty)
    , _mode(renderer::PolygonMode::PolygonMode_Triangle)
    , _frontFace(renderer::FrontFace::FrontFace_Clockwise)
    , _localTransform(false)
{
}

ModelHeader::~ModelHeader()
{
}


ModelHeader::GeometryInfo::GeometryInfo()
    : _count(0)
    , _size(0)
    , _present(false)
{
}

void ModelHeader::GeometryInfo::operator >> (stream::Stream * stream) const
{
    stream->write<u32>(_count);
    stream->write<u64>(_size);
    stream->write<bool>(_present);

    u32 count = static_cast<u32>(_subData.size());
    stream->write<u32>(count);
    stream->write(_subData.data(), count * sizeof(SubData), 1);
}

void ModelHeader::GeometryInfo::operator << (const stream::Stream * stream)
{
    stream->read<u32>(_count);
    stream->read<u64>(_size);
    stream->read<bool>(_present);

    u32 count;
    stream->read<u32>(count);
    _subData.resize(count);
    stream->read(_subData.data(), count * sizeof(SubData), 1);
}



Model::Model(ModelHeader* header) noexcept
    : Resource(header)
{
    LOG_DEBUG("Model constructor %xll", this);
}

Model::~Model()
{
    LOG_DEBUG("Model destructor %xll", this);
}

const ModelHeader& Model::getModelHeader() const
{
    return *(static_cast<const scene::ModelHeader*>(m_header));
}

Mesh * Model::getMeshByIndex(u32 index) const
{
    ASSERT(index < m_meshes.size(), "range out");
    return m_meshes[index];
}

u32 Model::getMeshCount() const
{
    return static_cast<u32>(m_meshes.size());
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

    u32 countMeshes = static_cast<u32>(header._meshes.size());
    for (u32 i = 0; i < countMeshes; ++i)
    {
        const MeshHeader& meshHeader = header._meshes[i];

        ASSERT(meshHeader._size > 0, "empty size");
        u32 size = static_cast<u32>(meshHeader._size) + sizeof(renderer::VertexInputAttribDescription);
        stream::Stream* meshStream = stream::StreamManager::createMemoryStream(nullptr, size);

        m_stream->seekBeg(static_cast<u32>(meshHeader._offset) + sizeof(renderer::VertexInputAttribDescription) * i);
        u8* data = m_stream->map(size);
        meshStream->write(data, size, 1);
        m_stream->unmap();

        MeshHeader* newMeshHeader = new MeshHeader(header._meshes[i]);

        Mesh* mesh = new Mesh(newMeshHeader);
        mesh->init(meshStream);

        if (!mesh->load())
        {
            ASSERT(false, "fail load mesh");

            delete mesh;
            continue;
        }

        m_meshes.push_back(mesh);
    }

    //TODO matterials

    ASSERT(!m_stream->isMapped(), "mapped");
    delete m_stream;
    m_stream = nullptr;

    m_loaded = true;
    return true;
}

} //namespace scene
} //namespace v3d
