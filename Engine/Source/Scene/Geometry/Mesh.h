#pragma once

#include "Common.h"

#include "Resource/Resource.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "AABB.h"

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
        enum GeometryContentFlag
        {
            Empty = 0,
            IndexBuffer = 1 << 0,
            BoundingBox = 1 << 1,
            SeparatePostionAttribute = 1 << 3
        };
        typedef u16 GeometryContentFlags;

        /**
        * @brief VertexProperies enum
        */
        enum VertexProperies : u16
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
        typedef u16 VertexProperiesFlags;

        MeshHeader() noexcept;
        ~MeshHeader() noexcept = default;

        u32 operator>>(stream::Stream* stream) override;
        u32 operator<<(const stream::Stream* stream) override;

        u32 _numVertices;
        u32 _vertexStride;
        u32 _numIndices;
        renderer::StreamIndexBufferType _indexType;

        renderer::PolygonMode _polygonMode;
        renderer::FrontFace _frontFace;
        VertexProperiesFlags _vertexContentFlags;
        GeometryContentFlags _geometryContentFlags;
    };

    class Mesh: public resource::Resource
    {
    public:

        explicit Mesh(const MeshHeader* header) noexcept;
        ~Mesh() noexcept;

        void init(stream::Stream* stream) override;
        bool load() override;

        const renderer::VertexInputAttributeDescription& getInputAttributeDesc() const;

        u32 getVertexCount() const;
        u64 getVertexSize(u32 stream = 0) const;
        const u8* getVertexData(u32 stream = 0) const;

        u32 getIndexCount() const;
        u64 getIndexSize() const;
        const u8* getIndexData() const;

        //AABB& getAABB() const;

    private:

        const MeshHeader& getMeshHeader() const;

        renderer::VertexInputAttributeDescription m_description;

        std::tuple<stream::Stream*, u8*> m_indexBuffer;
        u32 m_indexCount;
        renderer::StreamIndexBufferType m_indexType;

        std::vector<std::tuple<stream::Stream*, u8*>> m_vertexBuffers;
        u32 m_vertexCount;

        //AABB m_boundingBox;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace scene
} //namespace v3d