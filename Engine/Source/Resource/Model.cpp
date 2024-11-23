#include "Model.h"

#include "Stream/StreamManager.h"
#include "Mesh.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{


ModelResource::ModelResource() noexcept
    : m_header()
    , m_content(0)
{
    LOG_DEBUG("ModelResource constructor %llx", this);
}

ModelResource::ModelResource(const ModelHeader& header) noexcept
    : m_header(header)
    , m_content(0)
{
    LOG_DEBUG("ModelResource constructor %llx", this);
}

ModelResource::~ModelResource()
{
    LOG_DEBUG("ModelResource destructor %llx", this);

    //for (auto& meshLODs : m_meshes)
    //{
    //    for (auto& mesh : meshLODs)
    //    {
    //        V3D_DELETE(mesh, memory::MemoryLabel::MemoryObject);
    //    }
    //}

    //for (auto& material : m_materials)
    //{
    //    V3D_DELETE(material, memory::MemoryLabel::MemoryObject);
    //}
}

bool ModelResource::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("ModelResource::load: the model %llx is already loaded", this);
        return true;
    }
    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<ModelContentFlags>(m_content);
    if (m_content & ModelContent::ModelContent_Meshes)
    {
        u32 numMeshes = 0;
        stream->read<u32>(numMeshes);

        m_geometry.resize(numMeshes);
        for (auto& geometry : m_geometry)
        {
            u32 LODs = 0;
            stream->read<u32>(LODs);

            for (u32 LOD = 0; LOD < LODs; ++LOD)
            {
                resource::MeshResource::MeshHeader header;
                header << stream;
                resource::Resource* mesh = V3D_NEW(MeshResource, memory::MemoryLabel::MemoryObject)(header);

                geometry._LODs.resize(LODs, nullptr);

                if (!mesh->load(stream, stream->tell()))
                {
                    LOG_ERROR("ModelResource::load: The mesh can't be parsed");

                    ASSERT(false, "wrong parsing");
                    V3D_DELETE(mesh, memory::MemoryLabel::MemoryObject);

                    return false;
                }

                geometry._LODs[LOD] = static_cast<MeshResource*>(mesh);
            }
        }
    }
    
    //if (m_header->_modelContentFlags & ModelHeader::ModelContent::ModelContent_Material)
    //{
    //    u32 numMaterials = 0;
    //    stream->read<u32>(numMaterials);

    //    m_materials.resize(numMaterials, nullptr);
    //    for (auto& material : m_materials)
    //    {
    //        resource::Resource* resource = ::V3D_NEW(Material, memory::MemoryLabel::MemoryObject);
    //        if (!resource->load(stream, stream->tell()))
    //        {
    //            LOG_ERROR("Model::load: The material can't be parsed");

    //            ASSERT(false, "wrong parsing");
    //            V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);

    //            return false;
    //        }

    //        material = static_cast<Material*>(resource);
    //    }
    //}

    m_loaded = true;

    return true;
}

bool ModelResource::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

} //namespace resource
} //namespace v3d
