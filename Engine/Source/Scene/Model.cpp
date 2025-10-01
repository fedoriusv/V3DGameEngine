#include "Model.h"

#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

#include "Resource/Bitmap.h"
#include "Resource/Loader//ModelFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/ResourceManager.h"
#include "Stream/FileLoader.h"

#include "Material.h"
#include "Geometry/StaticMesh.h"

namespace v3d
{
namespace scene
{

Model::Model(renderer::Device* device) noexcept
    : m_header()
    , m_device(device)
{
    LOG_DEBUG("Model constructor %llx", this);
}

Model::Model(renderer::Device* device, const ModelHeader& header) noexcept
    : m_header(header)
    , m_device(device)
{
    LOG_DEBUG("Model constructor %llx", this);
}

Model::Model(const Model& model) noexcept
    : SceneNode(model)
    , m_header(model.m_header)
    , m_device(model.m_device)

    , m_meshes(model.m_meshes)
    , m_materials(model.m_materials)
{
    LOG_DEBUG("Model copy constructor %llx", this);
}

Model::~Model()
{
    LOG_DEBUG("Model destructor %llx", this);
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
    ASSERT(offset == m_header._offset, "wrong offset");

    u32 numMaterials = 0;
    stream->read<u32>(numMaterials);
    for (u32 i = 0; i < numMaterials; ++i)
    {
        Material::MaterialHeader header;
        header << stream;

        Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device, header);
        if (!static_cast<resource::Resource*>(material)->load(stream, header._offset))
        {
            V3D_DELETE(material, memory::MemoryLabel::MemoryObject);
            continue;
        }

        m_materials.push_back(material);
    }

    u32 numlights = 0;
    stream->read<u32>(numlights);
    for (u32 i = 0; i < numlights; ++i)
    {
        Light::LightHeader header;
        header << stream;

        Light* light = nullptr;
        Light::Type type = header.getResourceSubType<Light::Type>();
        switch (type)
        {
        case Light::Type::DirectionalLight:
            light = V3D_NEW(DirectionalLight, memory::MemoryLabel::MemoryObject)(m_device, header);
            break;

        case Light::Type::PointLight:
            light = V3D_NEW(PointLight, memory::MemoryLabel::MemoryObject)(m_device, header);
            break;

        case Light::Type::SpotLight:
            light = V3D_NEW(SpotLight, memory::MemoryLabel::MemoryObject)(m_device, header);
            break;
        }

        if (!static_cast<resource::Resource*>(light)->load(stream, header._offset))
        {
            V3D_DELETE(light, memory::MemoryLabel::MemoryObject);
            continue;
        }

        m_lights.push_back(light);
    }

    u32 numCameras = 0;
    stream->read<u32>(numCameras);
    for (u32 i = 0; i < numCameras; ++i)
    {
        Camera::CameraHeader header;
        header << stream;

        Camera* camera = V3D_NEW(Camera, memory::MemoryLabel::MemoryObject)(header);
        if (!static_cast<resource::Resource*>(camera)->load(stream, header._offset))
        {
            V3D_DELETE(camera, memory::MemoryLabel::MemoryObject);
            continue;
        }

        m_cameras.push_back(camera);
    }

    m_loaded = loadNode(this, stream, stream->tell());
    return true;
}

bool Model::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

bool Model::loadNode(SceneNode* node, const stream::Stream* stream, u32 offset)
{
    ASSERT(node, "node is nullptr");
    ASSERT(stream, "stream is nullptr");

    stream->read(node->m_name);

    math::Matrix4D tranform;
    stream->read<math::Matrix4D>(tranform);
    SceneNode::setTransform(TransformMode::Local, tranform);

    u32 numNodes = 0;
    stream->read<u32>(numNodes);

    bool hasLight = false;
    stream->read<bool>(hasLight);
    if (hasLight)
    {
        u32 lightID;
        stream->read<u32>(lightID);

        node->addComponent(m_lights[lightID]);
    }

    u32 numLODs = 0;
    stream->read<u32>(numLODs);
    for (u32 lod = 0; lod < numLODs; ++lod)
    {
        Mesh::MeshHeader header;
        header << stream;

        Mesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(m_device, header);
        if (!mesh->load(stream, header._offset))
        {
            V3D_DELETE(mesh, memory::MemoryLabel::MemoryObject);
            continue;
        }

        u32 materialID = 0;
        stream->read<u32>(materialID);

        node->addComponent(mesh);
        if (!m_materials.empty())
        {
            node->addComponent(m_materials[materialID]);
        }

        m_meshes.push_back(mesh);
    }

    for (u32 i = 0; i < numNodes; ++i)
    {
        SceneNode* child = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryObject)();
        if (!loadNode(child, stream, offset))
        {
            V3D_DELETE(child, memory::MemoryLabel::MemoryObject);
            continue;
        }

        node->addChild(child);
    }

    return node;
}

} //namespace resource
} //namespace v3d
