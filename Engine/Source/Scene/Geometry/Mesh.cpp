#include "Mesh.h"
#include "Resource/Mesh.h"
#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

Mesh::Mesh() noexcept
    : m_indexBuffer(nullptr)
    , m_topology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList)
{
    LOG_DEBUG("Mesh::Mesh constructor %llx", this);
}

Mesh::~Mesh()
{
    LOG_DEBUG("Mesh::Mesh destructor %llx", this);

    if (m_indexBuffer)
    {
        V3D_DELETE(m_indexBuffer, memory::MemoryLabel::MemoryObject);
        m_indexBuffer = nullptr;
    }

    for (auto& buffer : m_vertexBuffer)
    {
        V3D_DELETE(buffer, memory::MemoryLabel::MemoryObject);
    }
    m_vertexBuffer.clear();
}

StaticMesh::StaticMesh() noexcept
{
}

StaticMesh::~StaticMesh()
{
}

Mesh* MeshHelper::createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, resource::MeshResource* resource)
{
    ASSERT(resource && resource->m_loaded, "must be valid");
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = resource->m_description;
    mesh->m_name.assign(reinterpret_cast<const c8*>(resource->m_header._name));

    if (get<0>(resource->m_indexData._indexBuffer))
    {
        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, resource->getIndexBufferType(), resource->getIndexBufferCount(), "IndexBuffer");
        u32 dataSize = (resource->getIndexBufferType() == renderer::IndexBufferType::IndexType_32) ? sizeof(u32) : sizeof(u16);
        cmdList->uploadData(indexBuffer, 0, indexBuffer->getIndicesCount() * dataSize, resource->getIndexBufferData());
        mesh->m_indexBuffer = indexBuffer;
    }

    for (u32 index = 0; index < resource->m_vertexData._vertexBuffers.size(); ++index)
    {
        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, resource->getVertexBufferCount(), resource->getVertexBufferSize(), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, resource->getVertexBufferSize(), resource->getVertexBufferData());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    return mesh;
}

} //namespace scene
} //namespace v3d