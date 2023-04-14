#pragma once

#include "Common.h"

#include "Resource/Resource.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/PipelineStateProperties.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief MeshHeader meta info of Mesh
    */
    struct MeshHeader : resource::ResourceHeader
    {
        enum GeometryContentFlag : u32
        {
            Empty = 0,
            IndexBuffer = 1 << 0,
            BoundingBox = 1 << 1,
            SeparatePostion = 1 << 3
        };
        typedef u32 GeometryContentFlags;

        /**
        * @brief VertexProperies enum
        */
        enum VertexProperies : u32
        {
            VertexProperies_Empty = 0,
            VertexProperies_Position = 1 << 0,
            VertexProperies_Normals = 1 << 1,
            VertexProperies_Tangent = 1 << 2,
            VertexProperies_Bitangent = 1 << 3,
            VertexProperies_TextCoord0 = 1 << 4,
            VertexProperies_TextCoord1 = 1 << 5,
            VertexProperies_TextCoord2 = 1 << 6,
            VertexProperies_TextCoord3 = 1 << 7,
            VertexProperies_Color0 = 1 << 8,
            VertexProperies_Color1 = 1 << 9,
            VertexProperies_Color2 = 1 << 10,
            VertexProperies_Color3 = 1 << 11,
        };
        typedef u32 VertexProperiesFlags;

        MeshHeader() noexcept;
        ~MeshHeader() noexcept = default;

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

        u32 _numVertices;
        u32 _numIndices;
        u16 _vertexStride;
        renderer::StreamIndexBufferType _indexType;

        renderer::PolygonMode _polygonMode;
        renderer::FrontFace _frontFace;
        VertexProperiesFlags _vertexContentFlags;
        GeometryContentFlags _geometryContentFlags;
    };

    class Mesh: public resource::Resource
    {
    public:

        Mesh() noexcept;
        explicit Mesh(MeshHeader* header) noexcept;
        ~Mesh() noexcept;

        const renderer::VertexInputAttributeDescription& getInputAttributeDesc() const;

        u32 getVertexCount() const;
        u64 getVertexSize(u32 stream = 0) const;
        const void* getVertexData(u32 stream = 0) const;

        u32 getIndexCount() const;
        u64 getIndexSize() const;
        const void* getIndexData() const;

        //AABB& getAABB() const;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        MeshHeader* m_header;

        renderer::VertexInputAttributeDescription m_description;

        std::tuple<stream::Stream*, void*> m_indexBuffer;
        u32 m_indexCount;
        renderer::StreamIndexBufferType m_indexType;

        std::vector<std::tuple<stream::Stream*, void*>> m_vertexBuffers;
        u32 m_vertexCount;

        //AABB m_boundingBox;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace scene
} //namespace v3d
