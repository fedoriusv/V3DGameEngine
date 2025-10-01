#include "StaticMesh.h"
#include "Stream/StreamManager.h"
#include "Renderer/Device.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

StaticMesh::StaticMesh(renderer::Device* device) noexcept
    : ComponentBase<StaticMesh, Mesh>(device, Mesh::MeshType::Static)
{
}

StaticMesh::StaticMesh(renderer::Device* device, const MeshHeader& header) noexcept
    : ComponentBase<StaticMesh, Mesh>(device, header)
{
}

StaticMesh::~StaticMesh()
{
}

bool StaticMesh::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("StaticMesh::load: the texture %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "stream is nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    m_description << stream;
    stream->read<renderer::PrimitiveTopology>(m_topology);

    renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    u32 streamsCount;
    stream->read<u32>(streamsCount);
    for (u32 streamNo = 0; streamNo < streamsCount; ++streamNo)
    {
        u32 verticesCount;
        stream->read<u32>(verticesCount);

        u32 sizeInBytes;
        stream->read<u32>(sizeInBytes);
        void* data = stream->map(sizeInBytes);

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(m_device, renderer::BufferUsage::Buffer_GPUOnly, verticesCount, sizeInBytes, 
            std::string(m_header.getName()) + "_VertexBuffer_" + std::to_string(streamNo));
        m_vertexBuffer.push_back(vertexBuffer);

        cmdList->upload(vertexBuffer, 0, sizeInBytes, data);
        stream->unmap();
        stream->seekCur(sizeInBytes);
    }

    u32 indicesCount;
    stream->read<u32>(indicesCount);
    if (indicesCount > 0)
    {
        bool isIndexType32;
        stream->read<bool>(isIndexType32);
        renderer::IndexBufferType indexType = isIndexType32 ? renderer::IndexBufferType::IndexType_32 : renderer::IndexBufferType::IndexType_16;

        u32 sizeInBytes = indicesCount * (isIndexType32 ? sizeof(u32) : sizeof(u16));
        void* data = stream->map(sizeInBytes);

        m_indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(m_device, renderer::BufferUsage::Buffer_GPUOnly, indexType, indicesCount, 
            std::string(m_header.getName()) + "_IndexBuffer");

        cmdList->upload(m_indexBuffer, 0, sizeInBytes, data);
        stream->unmap();
        stream->seekCur(sizeInBytes);
    }

    stream->read<math::AABB>(m_boundingBox);

    m_device->submit(cmdList, true);
    m_device->destroyCommandList(cmdList);

    m_loaded = true;
    return true;
}

bool StaticMesh::save(stream::Stream* stream, u32 offset) const
{
    if (!m_loaded)
    {
        LOG_WARNING("StaticMesh::save: the mesh %llx is not loaded", this);
        return false;
    }

    m_description >> stream;
    stream->write<renderer::PrimitiveTopology>(m_topology);

    stream->write<u32>(static_cast<u32>(m_vertexBuffer.size()));
    for (u32 streamNo = 0; streamNo < m_vertexBuffer.size(); ++streamNo)
    {
        stream->write<u32>(m_vertexBuffer[streamNo]->getVerticesCount());
        //TODO stream data
    }

    if (m_indexBuffer)
    {
        stream->write<u32>(m_indexBuffer->getIndicesCount());
        stream->write<bool>(m_indexBuffer->getIndexBufferType() == renderer::IndexBufferType::IndexType_32 ? true : false);
        //TODO stream data
    }
    else
    {
        stream->write<u32>(0);
    }

    return true;
}

} //namespace scene
} //namespace v3d