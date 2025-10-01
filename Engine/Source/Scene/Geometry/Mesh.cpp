#include "Mesh.h"
#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"
#include "StaticMesh.h"
#include "RenderTechniques/VertexFormats.h"

namespace v3d
{
namespace scene
{

Mesh::Mesh(renderer::Device* device, MeshType type) noexcept
    : m_header(type)
    , m_device(device)
    , m_topology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList)

    , m_indexBuffer(nullptr)
{
    LOG_DEBUG("Mesh::Mesh constructor %llx", this);
}

Mesh::Mesh(renderer::Device* device, const MeshHeader& header) noexcept
    : m_header(header)
    , m_device(device)
    , m_topology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList)

    , m_indexBuffer(nullptr)
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


//Mesh* MeshHelper::createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, const resource::ModelResource* modelResource, u32 model, u32 LOD, const std::string& name)
//{
//    ASSERT(modelResource && modelResource->m_loaded, "must be valid");
//    ASSERT(!modelResource->m_geometry[model]._LODs.empty() && modelResource->m_geometry[model]._LODs[LOD], "must be valid");
//    const resource::ModelResource::MeshResource* meshResource = modelResource->m_geometry[model]._LODs[LOD];
//
//    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
//    mesh->m_description = meshResource->m_description;
//    mesh->m_name.assign(name.empty() ? meshResource->m_name : name);
//
//    if (get<0>(meshResource->m_indexData._indexBuffer))
//    {
//        void* data = std::get<1>(meshResource->m_indexData._indexBuffer);
//        u32 sizeType = (meshResource->m_indexData._indexType == renderer::IndexBufferType::IndexType_32) ? sizeof(u32) : sizeof(u16);
//
//        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, meshResource->m_indexData._indexType, meshResource->m_indexData._indexCount, "IndexBuffer");
//        cmdList->uploadData(indexBuffer, 0, indexBuffer->getIndicesCount() * sizeType, data);
//        mesh->m_indexBuffer = indexBuffer;
//    }
//
//    for (u32 index = 0; index < meshResource->m_vertexData._vertexBuffers.size(); ++index)
//    {
//        u32 sizeInBytes = std::get<0>(meshResource->m_vertexData._vertexBuffers[index])->size();
//        void* data = std::get<1>(meshResource->m_vertexData._vertexBuffers[index]);
//
//        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, meshResource->m_vertexData._vertexCount, sizeInBytes, "VertexBuffer");
//        cmdList->uploadData(vertexBuffer, 0, sizeInBytes, data);
//        mesh->m_vertexBuffer.push_back(vertexBuffer);
//    }
//
//    return mesh;
//}

Mesh* MeshHelper::createCube(renderer::Device* device, f32 extent, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    {
        static std::array<u32, 36> indices =
        {
            0, 1, 2,   0, 2, 3,     // Front
            4, 5, 6,   4, 6, 7,     // Back
            8, 9,10,   8,10,11,     // Left
            12,13,14,  12,14,15,    // Right
            16,17,18,  16,18,19,    // Top
            20,21,22,  20,22,23     // Bottom
        };

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        static std::array<scene::VertexFormatSimpleLit, 24> vertices =
        {
            // Front face (Z+)
            scene::VertexFormatSimpleLit{{-extent, -extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{-extent,  extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},

            // Back face (Z-)
            scene::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{-extent,  extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{-extent, -extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }},

            // Left face (X-)
            scene::VertexFormatSimpleLit{{-extent, -extent, -extent}, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{-extent,  extent, -extent}, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{-extent,  extent,  extent}, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{-extent, -extent,  extent}, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},

            // Right face (X+)
            scene::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},

            // Top face (Y+)
            scene::VertexFormatSimpleLit{{-extent,  extent,  extent}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{-extent,  extent, -extent}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},

            // Bottom face (Y-)
            scene::VertexFormatSimpleLit{{-extent, -extent, -extent}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
            scene::VertexFormatSimpleLit{{-extent, -extent,  extent}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},
            scene::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }},
        };

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);

        std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
            {
                bb->expand(vertex.position);
            });
    }

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

Mesh* MeshHelper::createSphere(renderer::Device* device, f32 radius, u32 stacks, u32 slices, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    {
        std::vector<u32> indices;
        for (u32 stack = 0; stack < stacks; ++stack)
        {
            for (u32 slice = 0; slice < slices; ++slice)
            {
                u32 first = (stack * (slices + 1)) + slice;
                u32 second = first + slices + 1;

                // Two triangles per quad
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        std::vector<scene::VertexFormatSimpleLit> vertices;
        for (u32 stack = 0; stack <= stacks; ++stack)
        {
            f32 v = (f32)stack / stacks;
            f32 phi = v * math::k_pi;

            for (u32 slice = 0; slice <= slices; ++slice)
            {
                f32 u = (f32)slice / slices;
                f32 theta = u * 2.0f * math::k_pi;

                f32 x = sinf(phi) * cosf(theta);
                f32 y = cosf(phi);
                f32 z = sinf(phi) * sinf(theta);

                scene::VertexFormatSimpleLit vertex;
                vertex.position[0] = radius * x;
                vertex.position[1] = radius * y;
                vertex.position[2] = radius * z;

                vertex.normal[0] = x;
                vertex.normal[1] = y;
                vertex.normal[2] = z;

                vertex.UV[0] = u;
                vertex.UV[1] = 1.0f - v;

                vertices.push_back(vertex);
            }
        }

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);

        std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
            {
                bb->expand(vertex.position);
            });
    }

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

Mesh* MeshHelper::createCone(renderer::Device* device, f32 radius, f32 height, u32 segments, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    std::vector<u32> indices;
    std::vector<scene::VertexFormatSimpleLit> vertices;
    u32 centerIndex = 0;

    {
        // Tip vertex (top)
        scene::VertexFormatSimpleLit tip = {};
        tip.position[0] = 0.0f;
        tip.position[1] = height;
        tip.position[2] = 0.0f;
        tip.normal[0] = 0.0f;
        tip.normal[1] = 1.0f;
        tip.normal[2] = 0.0f;
        tip.UV[0] = 0.5f;
        tip.UV[1] = 0.0f;
        u32 tipIndex = (u32)vertices.size();
        vertices.push_back(tip);

        // Side ring vertices
        for (u32 i = 0; i <= segments; ++i)
        {
            f32 theta = (f32)i / segments * 2.0f * math::k_pi;
            f32 x = cosf(theta);
            f32 z = sinf(theta);

            // Position on base circle
            f32 px = radius * x;
            f32 py = 0.0f;
            f32 pz = radius * z;

            // Normal (pointing halfway between side and up)
            f32 nx = x;
            f32 ny = radius / sqrtf(radius * radius + height * height);
            f32 nz = z;
            f32 len = sqrtf(nx * nx + ny * ny + nz * nz);
            nx /= len; ny /= len; nz /= len;

            f32 u = (f32)i / segments;
            f32 v = 1.0f;

            scene::VertexFormatSimpleLit vtx;
            vtx.position[0] = px;
            vtx.position[1] = py;
            vtx.position[2] = pz;
            vtx.normal[0] = nx;
            vtx.normal[1] = ny;
            vtx.normal[2] = nz;
            vtx.UV[0] = u;
            vtx.UV[1] = v;

            vertices.push_back(vtx);
        }

        // Side triangles (tip to base ring)
        for (u32 i = 1; i <= segments; ++i)
        {
            indices.push_back(tipIndex);
            indices.push_back(tipIndex + i);
            indices.push_back(tipIndex + i + 1);
        }

        // Center of base
        scene::VertexFormatSimpleLit center = {};
        center.position[0] = 0.0f;
        center.position[1] = 0.0f;
        center.position[2] = 0.0f;
        center.normal[0] = 0.0f;
        center.normal[1] = -1.0f;
        center.normal[2] = 0.0f;
        center.UV[0] = 0.5f;
        center.UV[1] = 0.5f;
        centerIndex = (u32)vertices.size();
        vertices.push_back(center);

        // Base circle vertices again (flat bottom)
        for (u32 i = 0; i <= segments; ++i)
        {
            f32 theta = (f32)i / segments * 2.0f * math::k_pi;
            f32 x = cosf(theta);
            f32 z = sinf(theta);

            f32 px = radius * x;
            f32 py = 0.0f;
            f32 pz = radius * z;

            scene::VertexFormatSimpleLit vtx;
            vtx.position[0] = px;
            vtx.position[1] = py;
            vtx.position[2] = pz;
            vtx.normal[0] = 0.0f;
            vtx.normal[1] = -1.0f;
            vtx.normal[2] = 0.0f;
            vtx.UV[0] = 0.5f + x * 0.5f;
            vtx.UV[1] = 0.5f + z * 0.5f;

            vertices.push_back(vtx);
        }

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);

        std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
            {
                bb->expand(vertex.position);
            });
    }

    {
        for (u32 i = 1; i <= segments; ++i)
        {
            indices.push_back(centerIndex);
            indices.push_back(centerIndex + i + 1);
            indices.push_back(centerIndex + i);
        }

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

Mesh* MeshHelper::createCylinder(renderer::Device* device, f32 radius, f32 height, u32 segments, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    std::vector<u32> indices;
    std::vector<scene::VertexFormatSimpleLit> vertices;

    f32 halfHeight = height * 0.5f;

    // ===== Side wall =====
    for (u32 i = 0; i <= segments; ++i)
    {
        float theta = (float)i / segments * 2.0f * math::k_pi;
        float x = cosf(theta);
        float z = sinf(theta);
        float u = (float)i / segments;

        scene::VertexFormatSimpleLit vTop = {
            { radius * x, +halfHeight, radius * z },
            { x, 0.0f, z },
            { u, 0.0f }
        };

        scene::VertexFormatSimpleLit vBottom = {
            { radius * x, -halfHeight, radius * z },
            { x, 0.0f, z },
            { u, 1.0f }
        };

        vertices.push_back(vTop);
        vertices.push_back(vBottom);
    }

    // -------- Side Indices --------
    for (u32 i = 0; i < segments; ++i)
    {
        u32 base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }


    // -------- Top Cap --------
    uint32_t topCenterIndex = (uint32_t)vertices.size();
    vertices.push_back({
        { 0.0f, +halfHeight, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.5f, 0.5f }
        });

    for (u32 i = 0; i <= segments; ++i)
    {
        float theta = (float)i / segments * 2.0f * math::k_pi;
        float x = cosf(theta);
        float z = sinf(theta);

        vertices.push_back({
            { radius * x, +halfHeight, radius * z },
            { 0.0f, 1.0f, 0.0f },
            { 0.5f + x * 0.5f, 0.5f - z * 0.5f }
            });
    }

    for (u32 i = 0; i < segments; ++i)
    {
        indices.push_back(topCenterIndex);
        indices.push_back(topCenterIndex + i + 1);
        indices.push_back(topCenterIndex + i + 2);
    }

    // -------- Bottom Cap --------
    uint32_t bottomCenterIndex = (uint32_t)vertices.size();
    vertices.push_back({
        { 0.0f, -halfHeight, 0.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.5f, 0.5f }
        });

    for (u32 i = 0; i <= segments; ++i)
    {
        float theta = (float)i / segments * 2.0f * math::k_pi;
        float x = cosf(theta);
        float z = sinf(theta);

        vertices.push_back({
            { radius * x, -halfHeight, radius * z },
            { 0.0f, -1.0f, 0.0f },
            { 0.5f + x * 0.5f, 0.5f + z * 0.5f }
            });
    }

    for (u32 i = 0; i < segments; ++i)
    {
        indices.push_back(bottomCenterIndex);
        indices.push_back(bottomCenterIndex + i + 2);
        indices.push_back(bottomCenterIndex + i + 1);
    }

    renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
    cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
    mesh->m_indexBuffer = indexBuffer;

    renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
    cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
    mesh->m_vertexBuffer.push_back(vertexBuffer);

    std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
        {
            bb->expand(vertex.position);
        });

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

Mesh* MeshHelper::createPlane(renderer::Device* device, f32 width, f32 height, u32 segmentsX, u32 segmentsY, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    {
        std::vector<u32> indices;
        for (u32 z = 0; z < segmentsY; ++z)
        {
            for (u32 x = 0; x < segmentsX; ++x)
            {
                u32 start = z * (segmentsX + 1) + x;
                indices.push_back(start);
                indices.push_back(start + segmentsX + 1);
                indices.push_back(start + 1);

                indices.push_back(start + 1);
                indices.push_back(start + segmentsX + 1);
                indices.push_back(start + segmentsX + 2);
            }
        }

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        f32 halfWidth = width * 0.5f;
        f32 halfDepth = height * 0.5f;
        std::vector<scene::VertexFormatSimpleLit> vertices;
        for (u32 z = 0; z <= segmentsY; ++z)
        {
            f32 v = (f32)z / segmentsY;
            f32 zPos = height * v - halfDepth;

            for (u32 x = 0; x <= segmentsX; ++x)
            {
                f32 u = (f32)x / segmentsX;
                f32 xPos = width * u - halfWidth;

                scene::VertexFormatSimpleLit vert;
                vert.position[0] = xPos;
                vert.position[1] = 0.0f;
                vert.position[2] = zPos;

                vert.normal[0] = 0.0f;
                vert.normal[1] = 1.0f;
                vert.normal[2] = 0.0f;

                vert.UV[0] = u;
                vert.UV[1] = 1.0f - v;

                vertices.push_back(vert);
            }
        }

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);

        std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
            {
                bb->expand(vertex.position);
            });
    }

    return mesh;
}

Mesh* MeshHelper::createGrid(renderer::Device* device, f32 cellSize, u32 rows, u32 cols, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);
    mesh->m_topology = renderer::PrimitiveTopology::PrimitiveTopology_LineList;

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    {
        std::vector<u32> indices;
        for (u32 z = 0; z < rows; ++z)
        {
            for (u32 x = 0; x < cols; ++x)
            {
                u32 start = z * (cols + 1) + x;

                u32 i0 = start;
                u32 i1 = start + 1;
                u32 i2 = start + (cols + 1);
                u32 i3 = start + (cols + 1) + 1;

                // Triangle 1
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                // Triangle 2
                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        std::vector<scene::VertexFormatSimpleLit> vertices;

        f32 dx = cellSize / cols;
        f32 dz = cellSize / rows;
        f32 startX = -cellSize * 0.5f;
        f32 startZ = -cellSize * 0.5f;

        // Vertices
        for (int z = 0; z <= rows; ++z)
        {
            for (int x = 0; x <= cols; ++x)
            {
                f32 px = startX + x * dx;
                f32 pz = startZ + z * dz;

                scene::VertexFormatSimpleLit v;
                v.position[0] = px;
                v.position[1] = 0.0f;
                v.position[2] = pz;

                v.normal[0] = 0.0f;
                v.normal[1] = 1.0f;
                v.normal[2] = 0.0f;

                v.UV[0] = static_cast<f32>(x) / cols;
                v.UV[1] = static_cast<f32>(z) / rows;

                vertices.push_back(v);
            }
        }

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(scene::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);

        std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
            {
                bb->expand(vertex.position);
            });
    }

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

Mesh* MeshHelper::createLineSegment(renderer::Device* device, const std::vector<math::float3>& points, const std::string& name)
{
    if (points.empty())
    {
        return nullptr;
    }
    ASSERT(points.size() > 1, "must be 2 or more");

    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)(device);
    mesh->m_description = scene::VertexFormatSimpleLitDesc;
    mesh->m_header.setName(name);
    mesh->m_topology = renderer::PrimitiveTopology::PrimitiveTopology_LineList;

    std::vector<u32> indices;
    std::vector<scene::VertexFormatSimpleLit> vertices;

    for (u32 index = 0; index < points.size(); ++index)
    {
        scene::VertexFormatSimpleLit v;
        v.position = points[index];
        v.normal = { 0.f, 1.f, 0.f };
        v.UV = { 0.f, 0.f };

        vertices.push_back(v);
        indices.push_back(index);
    }

    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    u32 countLines = points.size() - 1;
    renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
    cmdList->upload(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
    mesh->m_indexBuffer = indexBuffer;

    renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, countLines, vertices.size() * sizeof(scene::VertexFormatSimpleLit), "VertexBuffer");
    cmdList->upload(vertexBuffer, 0, vertices.size() * sizeof(scene::VertexFormatSimpleLit), vertices.data());
    mesh->m_vertexBuffer.push_back(vertexBuffer);

    std::for_each(vertices.cbegin(), vertices.cend(), [bb = &mesh->m_boundingBox](const scene::VertexFormatSimpleLit& vertex)
        {
            bb->expand(vertex.position);
        });

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return mesh;
}

} //namespace scene
} //namespace v3d