#include "Model.h"

#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

#include "Resource/Bitmap.h"
#include "Resource/Loader//ModelFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/ResourceManager.h"
#include "Stream/FileLoader.h"

namespace v3d
{
namespace resource
{


ModelResource::ModelResource() noexcept
    : m_header()
    , m_contentFlags(0)
{
    LOG_DEBUG("ModelResource constructor %llx", this);
}

ModelResource::ModelResource(const ModelHeader& header) noexcept
    : m_header(header)
    , m_contentFlags(0)
{
    LOG_DEBUG("ModelResource constructor %llx", this);
}

ModelResource::~ModelResource()
{
    LOG_DEBUG("ModelResource destructor %llx", this);

    for (auto& meshLODs : m_geometry)
    {
        for (auto& mesh : meshLODs._LODs)
        {
            if (std::get<0>(mesh->m_indexData._indexBuffer))
            {
                stream::StreamManager::destroyStream(std::get<0>(mesh->m_indexData._indexBuffer));
            }
            mesh->m_indexData._indexBuffer = {};

            for (auto& stream : mesh->m_vertexData._vertexBuffers)
            {
                std::get<0>(stream)->unmap();
                stream::StreamManager::destroyStream(std::get<0>(stream));
            }
            mesh->m_vertexData._vertexBuffers.clear();

            V3D_DELETE(mesh, memory::MemoryLabel::MemoryObject);
        }
        meshLODs._LODs.clear();
    }
    m_geometry.clear();

    for (auto& material : m_materials)
    {
        V3D_DELETE(material, memory::MemoryLabel::MemoryObject);
    }
    m_materials.clear();

    for (auto& image : m_images)
    {
        resource::ResourceManager::getInstance()->remove(image);
    }
    m_images.clear();
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

    stream->read<ModelContentFlags>(m_contentFlags);
    if (m_contentFlags & ModelContent::ModelContent_Meshes)
    {
        u32 numMeshes = 0;
        stream->read<u32>(numMeshes);

        m_geometry.resize(numMeshes);
        for (auto& geometry : m_geometry)
        {
            u32 LODs = 0;
            stream->read<u32>(LODs);

            geometry._LODs.resize(LODs, nullptr);
            geometry._boundingBox.resize(LODs);
            for (u32 LOD = 0; LOD < LODs; ++LOD)
            {
                geometry._LODs[LOD] = ModelResource::loadMesh(stream, offset);
                stream->read<math::AABB>(geometry._boundingBox[LOD]);
            }
        }
    }
    
    if (m_contentFlags & ModelContent::ModelContent_Materials)
    {
        u32 numMaterials = 0;
        stream->read<u32>(numMaterials);

        m_materials.resize(numMaterials);
        for (auto& material : m_materials)
        {
            material = ModelResource::loadMaterial(stream, offset);
        }

        u32 numTextures = 0;
        stream->read<u32>(numTextures);

        m_images.resize(numTextures, nullptr);
        for (auto& image : m_images)
        {
            std::string id;
            stream->read(id);

            if (!id.empty())
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(id);

                resource::ImageLoaderFlags flags = 0;
                resource::Bitmap* bitmap = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>(id, flags);
                image = bitmap;
            }
        }
    }

    m_loaded = true;

    return true;
}

bool ModelResource::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

ModelResource::MeshResource* ModelResource::loadMesh(const stream::Stream* stream, u32 offset)
{
    ASSERT(stream, "stream is nullptr");
    ModelResource::MeshResource* mesh = V3D_NEW(ModelResource::MeshResource, memory::MemoryLabel::MemoryObject)();

    stream->read(mesh->m_name);
    mesh->m_description << stream;

    u32 streamsCount;
    stream->read<u32>(streamsCount);
    for (u32 streamNo = 0; streamNo < streamsCount; ++streamNo)
    {
        stream->read<u32>(mesh->m_vertexData._vertexCount);

        u32 streamSize;
        stream->read<u32>(streamSize);
        stream::Stream* vertexData = stream::StreamManager::createMemoryStream(nullptr, streamSize);
        void* ptr = vertexData->map(vertexData->size());
        stream->read(ptr, vertexData->size());

        mesh->m_vertexData._vertexBuffers.push_back(std::make_tuple(vertexData, ptr));
    }

    stream->read<u32>(mesh->m_indexData._indexCount);
    if (mesh->m_indexData._indexCount > 0)
    {
        bool isIndexType32;
        stream->read<bool>(isIndexType32);
        mesh->m_indexData._indexType = isIndexType32 ? renderer::IndexBufferType::IndexType_32 : renderer::IndexBufferType::IndexType_16;

        u32 streamSize = mesh->m_indexData._indexCount * (isIndexType32 ? sizeof(u32) : sizeof(u16));
        stream::Stream* indexData = stream::StreamManager::createMemoryStream(nullptr, streamSize);
        void* ptr = indexData->map(streamSize);
        stream->read(ptr, streamSize);

        mesh->m_indexData._indexBuffer = std::make_tuple(indexData, ptr);
    }

    return mesh;
}

ModelResource::MaterialResource* ModelResource::loadMaterial(const stream::Stream* stream, u32 offset)
{
    ASSERT(stream, "stream is nullptr");
    ModelResource::MaterialResource* material = V3D_NEW(ModelResource::MaterialResource, memory::MemoryLabel::MemoryObject)();

    enum PropertyType : u8
    {
        Scalar = 0,
        Vector = 1,
        TextureID = 2
    };

    static auto propertyValue = [](const stream::Stream* stream, PropertyType type) -> MaterialResource::Property
        {
            switch (type)
            {
            default:
            case Scalar:
            {
                f32 value = 0.f;
                stream->read<f32>(value);
                return value;
            }
            case Vector:
            {
                math::float4 value;
                stream->read<math::float4>(value);
                return value;
            }
            case TextureID:
            {
                u32 id = 0;
                stream->read<u32>(id);
                return id;
            }
            }
        };

    u32 properties = 0;
    stream->read(material->m_name);
    stream->read<u32>(properties);

    for (u32 i = 0; i < properties; ++i)
    {
        std::string propertyName;
        PropertyType type;
        MaterialResource::Property property;

        stream->read(propertyName);
        stream->read<PropertyType>(type);
        property = propertyValue(stream, type);

        material->m_properties.emplace_back(propertyName, property);
    }

    return material;
}

} //namespace resource
} //namespace v3d
