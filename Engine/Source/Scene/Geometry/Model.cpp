#include "Model.h"
#include "Stream/StreamManager.h"
#include "Scene/Geometry.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

ModelHeader::ModelHeader() noexcept
    : _numMeshes(0)
    , _numMaterials(0)
    , _modelContentFlags(ModelContent::ModelContent_Empty)
    , _localTransform(false)
{
}

u32 ModelHeader::operator>>(stream::Stream* stream)
{
    u32 write = ResourceHeader::operator>>(stream);

    write += stream->write<u32>(_numMeshes);
    write += stream->write<u32>(_numMaterials);
    write += stream->write<u16>(_modelContentFlags);
    write += stream->write<bool>(_localTransform);

    return write;
}

u32 ModelHeader::operator<<(const stream::Stream* stream)
{
    u32 read = ResourceHeader::operator<<(stream);

    read += stream->read<u32>(_numMeshes);
    read += stream->read<u32>(_numMaterials);
    read += stream->read<u16>(_modelContentFlags);
    read += stream->read<bool>(_localTransform);

    return read;
}


Model::Model() noexcept
    : Resource()
{
    LOG_DEBUG("Model constructor %xll", this);
}

Model::Model(const ModelHeader* header) noexcept
    : Resource(header)
{
    LOG_DEBUG("Model constructor %xll", this);
}

Model::~Model()
{
    LOG_DEBUG("Model destructor %xll", this);
    //for (auto mesh : m_meshes)
    //{
    //    delete mesh;
    //}

    //for (auto material : m_materials)
    //{
    //    delete material;
    //}
}

const ModelHeader& Model::getModelHeader() const
{
    return *(static_cast<const scene::ModelHeader*>(m_header));
}

std::vector<Mesh*> Model::getMeshes(u32 LOD) const
{
    ASSERT(LOD < m_meshes.size(), "reange out");
    return m_meshes[LOD];
}

Mesh* Model::getMeshByIndex(u32 index, u32 LOD) const
{
    ASSERT(LOD < m_meshes.size(), "reange out");
    ASSERT(index < m_meshes[LOD].size(), "range out");
    return m_meshes[LOD][index];
}

u32 Model::getMeshCount() const
{
    if (m_meshes.empty())
    {
        return 0;
    }

    return static_cast<u32>(m_meshes[0].size());
}

std::vector<Material*> Model::getMaterials() const
{
    return m_materials;
}

Material* Model::getMaterialByIndex(u32 index) const
{
    ASSERT(index < m_materials.size(), "range out");
    return m_materials[index];
}

u32 Model::getMaterialsCount() const
{
    return static_cast<u32>(m_materials.size());
}


void Model::init(stream::Stream* stream)
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

    if (!m_header)
    {
        //m_header = new ModelHeader();
        //m_header >> m_stream;
    }
    //const ModelHeader& header = Model::getModelHeader();
    

   /* u32 countMeshes = static_cast<u32>(header._meshes.size());
    for (u32 i = 0; i < countMeshes; ++i)
    {
        const MeshHeader& meshHeader = header._meshes[i];
        ASSERT(meshHeader._size > 0, "empty size");

        Mesh* mesh = new Mesh(new MeshHeader(header._meshes[i]));
        mesh->init(m_stream);

        if (!mesh->load())
        {
            ASSERT(false, "fail load mesh");

            delete mesh;
            continue;
        }

        m_meshes.push_back(mesh);*/
    //}

    //u32 countMaterials = static_cast<u32>(header._materials.size());
   /* for (u32 i = 0; i < countMaterials; ++i)
    {
        MaterialHeader* newMaterialHeader = new MaterialHeader(header._materials[i]);

        Material* material = new Material(newMaterialHeader);
        material->init(nullptr);

        if (!material->load())
        {
            ASSERT(false, "fail load material");

            delete material;
            continue;
        }

        m_materials.push_back(material);
    }*/

    //ASSERT(!m_stream->isMapped(), "mapped");
    //delete m_stream;
    //m_stream = nullptr;

    //m_loaded = true;
    return true;
}

} //namespace scene
} //namespace v3d
