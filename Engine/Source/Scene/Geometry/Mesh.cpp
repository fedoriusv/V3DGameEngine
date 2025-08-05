#include "Mesh.h"
#include "Resource/Mesh.h"
#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"
#include "StaticMesh.h"
#include "RenderTechniques/VertexFormats.h"

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


Mesh* MeshHelper::createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, resource::MeshResource* resource, const std::string& name)
{
    ASSERT(resource && resource->m_loaded, "must be valid");
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = resource->m_description;
    mesh->m_name.assign(name.empty() ? reinterpret_cast<const c8*>(resource->m_header._name) : name);

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

Mesh* MeshHelper::createCube(renderer::Device* device, renderer::CmdListRender* cmdList, f32 extent, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;

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
        cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        static std::array<renderer::VertexFormatSimpleLit, 24> vertices =
        {
            // Front face (Z+)
            renderer::VertexFormatSimpleLit{{-extent, -extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{-extent,  extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},

            // Back face (Z-)
            renderer::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{-extent,  extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{-extent, -extent, -extent}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }},

            // Left face (X-)
            renderer::VertexFormatSimpleLit{{-extent, -extent, -extent}, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{-extent,  extent, -extent}, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{-extent,  extent,  extent}, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{-extent, -extent,  extent}, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},

            // Right face (X+)
            renderer::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},

            // Top face (Y+)
            renderer::VertexFormatSimpleLit{{-extent,  extent,  extent}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{-extent,  extent, -extent}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent, -extent}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent,  extent,  extent}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},

            // Bottom face (Y-)
            renderer::VertexFormatSimpleLit{{-extent, -extent, -extent}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
            renderer::VertexFormatSimpleLit{{-extent, -extent,  extent}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent, -extent,  extent}, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }},
            renderer::VertexFormatSimpleLit{{ extent, -extent, -extent}, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }},
        };

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    return mesh;
}

Mesh* MeshHelper::createSphere(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, u32 stacks, u32 slices, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;

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
        cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        std::vector<renderer::VertexFormatSimpleLit> vertices;
        for (u32 stack = 0; stack <= stacks; ++stack)
        {
            f32 v = (f32)stack / stacks;
            f32 phi = v * math::k_pi;

            for (int slice = 0; slice <= slices; ++slice)
            {
                f32 u = (f32)slice / slices;
                f32 theta = u * 2.0f * math::k_pi;

                f32 x = sinf(phi) * cosf(theta);
                f32 y = cosf(phi);
                f32 z = sinf(phi) * sinf(theta);

                renderer::VertexFormatSimpleLit vertex;
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

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    return mesh;
}

Mesh* MeshHelper::createCone(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, f32 height, u32 segments, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;

    std::vector<u32> indices;
    std::vector<renderer::VertexFormatSimpleLit> vertices;
    u32 centerIndex = 0;

    {
        // Tip vertex (top)
        renderer::VertexFormatSimpleLit tip = {};
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

            renderer::VertexFormatSimpleLit vtx;
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
        for (int i = 1; i <= segments; ++i)
        {
            indices.push_back(tipIndex);
            indices.push_back(tipIndex + i);
            indices.push_back(tipIndex + i + 1);
        }

        // Center of base
        renderer::VertexFormatSimpleLit center = {};
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
        for (int i = 0; i <= segments; ++i)
        {
            f32 theta = (f32)i / segments * 2.0f * math::k_pi;
            f32 x = cosf(theta);
            f32 z = sinf(theta);

            f32 px = radius * x;
            f32 py = 0.0f;
            f32 pz = radius * z;

            renderer::VertexFormatSimpleLit vtx;
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

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    {
        for (u32 i = 1; i <= segments; ++i)
        {
            indices.push_back(centerIndex);
            indices.push_back(centerIndex + i + 1);
            indices.push_back(centerIndex + i);
        }

        renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
        cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    return mesh;
}

Mesh* MeshHelper::createCylinder(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, f32 height, u32 segments, const std::string& name)
{
    return nullptr;
}

Mesh* MeshHelper::createPlane(renderer::Device* device, renderer::CmdListRender* cmdList, f32 width, f32 height, u32 segmentsX, u32 segmentsY, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;

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
        cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        f32 halfWidth = width * 0.5f;
        f32 halfDepth = height * 0.5f;
        std::vector<renderer::VertexFormatSimpleLit> vertices;
        for (u32 z = 0; z <= segmentsY; ++z)
        {
            f32 v = (f32)z / segmentsY;
            f32 zPos = height * v - halfDepth;

            for (u32 x = 0; x <= segmentsX; ++x)
            {
                f32 u = (f32)x / segmentsX;
                f32 xPos = width * u - halfWidth;

                renderer::VertexFormatSimpleLit vert;
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

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    return mesh;
}

Mesh* MeshHelper::createGrid(renderer::Device* device, renderer::CmdListRender* cmdList, f32 cellSize, u32 rows, u32 cols, const std::string& name)
{
    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;
    mesh->m_topology = renderer::PrimitiveTopology::PrimitiveTopology_LineList;

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
        cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
        mesh->m_indexBuffer = indexBuffer;
    }

    {
        std::vector<renderer::VertexFormatSimpleLit> vertices;

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

                renderer::VertexFormatSimpleLit v;
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

        renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
        cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
        mesh->m_vertexBuffer.push_back(vertexBuffer);
    }

    return mesh;
}

Mesh* MeshHelper::createLine(renderer::Device* device, renderer::CmdListRender* cmdList, const std::vector<math::float3>& points, const std::string& name)
{
    if (points.empty())
    {
        return nullptr;
    }

    StaticMesh* mesh = V3D_NEW(StaticMesh, memory::MemoryLabel::MemoryObject)();
    mesh->m_description = renderer::VertexFormatSimpleLitDesc;
    mesh->m_name = name;
    mesh->m_topology = renderer::PrimitiveTopology::PrimitiveTopology_LineList;

    std::vector<u32> indices;
    std::vector<renderer::VertexFormatSimpleLit> vertices;


    for (u32 index = 0; index < points.size(); ++index)
    {
        renderer::VertexFormatSimpleLit v;
        v.position = points[index];
        v.normal = { 0.f, 1.f, 0.f };
        v.UV = { 0.f, 0.f };

        vertices.push_back(v);
        indices.push_back(index);
    }

    renderer::IndexBuffer* indexBuffer = V3D_NEW(renderer::IndexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::Buffer_GPUOnly, renderer::IndexBufferType::IndexType_32, indices.size(), "IndexBuffer");
    cmdList->uploadData(indexBuffer, 0, indices.size() * sizeof(u32), indices.data());
    mesh->m_indexBuffer = indexBuffer;

    renderer::VertexBuffer* vertexBuffer = V3D_NEW(renderer::VertexBuffer, memory::MemoryLabel::MemoryObject)(device, renderer::BufferUsage::Buffer_GPUOnly, vertices.size(), vertices.size() * sizeof(renderer::VertexFormatSimpleLitDesc), "VertexBuffer");
    cmdList->uploadData(vertexBuffer, 0, vertices.size() * sizeof(renderer::VertexFormatSimpleLit), vertices.data());
    mesh->m_vertexBuffer.push_back(vertexBuffer);

    return mesh;
}

} //namespace scene
} //namespace v3d