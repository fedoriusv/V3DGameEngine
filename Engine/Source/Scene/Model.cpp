#include "Model.h"

#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

ModelHeader::ModelHeader() noexcept
    : ResourceHeader(resource::ResourceType::ModelResource)
    , _modelContentFlags(ModelContent::ModelContent_Empty)
    , _localTransform(0)
{
    static_assert(sizeof(ModelHeader) == sizeof(ResourceHeader) + 8, "wrong size");
}

ModelHeader::ModelHeader(const ModelHeader& other) noexcept
    : ResourceHeader(other)
    , _modelContentFlags(other._modelContentFlags)
    , _localTransform(other._localTransform)
{
}

u32 ModelHeader::operator>>(stream::Stream* stream) const
{
    u32 writeSize = ResourceHeader::operator>>(stream);
    u32 writePos = stream->tell();

    stream->write<ModelContentFlags>(_modelContentFlags);
    stream->write<u32>(_localTransform);

    writeSize = stream->tell() - writePos + writeSize;
    ASSERT(sizeof(ModelHeader) == writeSize, "wrong size");
    return writeSize;
}

u32 ModelHeader::operator<<(const stream::Stream* stream)
{
    u32 readSize = ResourceHeader::operator<<(stream);
    u32 readPos = stream->tell();

    stream->read<ModelContentFlags>(_modelContentFlags);
    stream->read<u32>(_localTransform);

    readPos = stream->tell() - readPos + readSize;
    ASSERT(sizeof(ModelHeader) == readSize, "wrong size");
    return readSize;
}


Model::Model() noexcept
    : m_header(nullptr)
{
    LOG_DEBUG("Model constructor %llx", this);
}

Model::Model(ModelHeader* header) noexcept
    : m_header(header)
{
    LOG_DEBUG("Model constructor %llx", this);
}

Model::~Model()
{
    LOG_DEBUG("Model destructor %llx", this);

    for (auto& meshLODs : m_meshes)
    {
        for (auto& mesh : meshLODs)
        {
            V3D_DELETE(mesh, memory::MemoryLabel::MemoryObject);
        }
    }

    for (auto& material : m_materials)
    {
        V3D_DELETE(material, memory::MemoryLabel::MemoryObject);
    }

    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryObject);
        m_header = nullptr;
    }
}

std::vector<Mesh*> Model::getMesh(u32 index) const
{
    ASSERT(index < m_meshes.size(), "reange out");
    return m_meshes[index];
}

Mesh* Model::getMeshByIndex(u32 index, u32 LOD) const
{
    ASSERT(index < m_meshes.size(), "reange out");
    ASSERT(LOD < m_meshes[index].size(), "range out");
    return m_meshes[index][LOD];
}

u32 Model::getMeshCount() const
{
    return static_cast<u32>(m_meshes.size());
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

bool Model::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Model::load: the model %llx is already loaded", this);
        return true;
    }
    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);

    if (!m_header)
    {
        m_header = V3D_NEW(ModelHeader, memory::MemoryLabel::MemoryObject);
        ASSERT(m_header, "nullptr");
        m_header->operator<<(stream);
    }
    stream->seekBeg(m_header->_offset);

    if (m_header->_modelContentFlags & ModelHeader::ModelContent::ModelContent_Mesh)
    {
        u32 numMeshes = 0;
        stream->read<u32>(numMeshes);

        m_meshes.resize(numMeshes);
        for (auto& mesh : m_meshes)
        {
            u32 LODs = 0;
            stream->read<u32>(LODs);

            mesh.resize(LODs, nullptr);
            for (u32 LOD = 0; LOD < LODs; ++LOD)
            {
                resource::Resource* resource = ::V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject);
                if (!resource->load(stream, stream->tell()))
                {
                    LOG_ERROR("Model::load: The mesh can't be parsed");

                    ASSERT(false, "wrong parsing");
                    V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);

                    return false;
                }

                mesh[LOD] = static_cast<Mesh*>(resource);
            }
        }
    }
    
    if (m_header->_modelContentFlags & ModelHeader::ModelContent::ModelContent_Material)
    {
        u32 numMaterials = 0;
        stream->read<u32>(numMaterials);

        m_materials.resize(numMaterials, nullptr);
        for (auto& material : m_materials)
        {
            resource::Resource* resource = ::V3D_NEW(Material, memory::MemoryLabel::MemoryObject);
            if (!resource->load(stream, stream->tell()))
            {
                LOG_ERROR("Model::load: The material can't be parsed");

                ASSERT(false, "wrong parsing");
                V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);

                return false;
            }

            material = static_cast<Material*>(resource);
        }
    }

    m_loaded = true;
    return true;
}

bool Model::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

} //namespace scene
} //namespace v3d
