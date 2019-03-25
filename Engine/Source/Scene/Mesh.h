#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"
#include "Renderer/BufferProperties.h"

namespace v3d
{
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * MeshHeader meta info about Mesh
    */
    struct MeshHeader : resource::ResourceHeader
    {
        MeshHeader();

        struct GeometryInfo
        {
            GeometryInfo();

            struct SubData
            {
                u64 _offset;
                u64 _size;
                u32 _count;
            };

            u64                  _size;
            u32                  _count;
            std::vector<SubData> _subData;

            void operator >> (stream::Stream * stream) const;
            void operator << (const stream::Stream * stream);
        };

        GeometryInfo    _index;
        GeometryInfo    _vertex;

        u64             _size;
        u64             _offset;

        bool            _indexPresent;
    };

    /**
    * Mesh class. Component, Resource
    */
    class Mesh : public Component, public resource::Resource
    {
    public:

        explicit Mesh(MeshHeader* header) noexcept;
        ~Mesh();

        void init(stream::Stream* stream) override;
        bool load() override;

        const renderer::VertexInputAttribDescription& getVertexInputAttribDesc() const;

        u8* getVertexData(s32 subMesh = -1) const;
        u32 getVertexCount(s32 subMesh = -1) const;
        u64 getVertexSize(s32 subMesh = -1) const;

        u8* getIndexData(s32 subMesh = -1) const;
        u32 getIndexCount(s32 subMesh = -1) const;
        u64 getIndexSize(s32 subMesh = -1) const;

    private:

        struct BufferData
        {
            u8* _data;
            u64 _size;
        };

        class SubMesh final
        {
        public:

            SubMesh() noexcept;
            ~SubMesh();

            u8* getVertexData() const;
            u32 getVertexCount() const;
            u64 getVertexSize() const;

            u8* getIndexData() const;
            u32 getIndexCount() const;
            u64 getIndexSize() const;

        private:

            void fillVertexData(u32 count, u8* data, u64 size);
            void fillIndexData(u32 count, u8* data, u64 size);

            u32 m_indexCount;
            BufferData m_indexData;

            u32 m_vertexCount;
            BufferData m_vertexData;

            friend Mesh;
        };

        const MeshHeader& getMeshHeader() const;
        void fillVertexData(u32 count, u8* data, u64 size);
        void fillIndexData(u32 count, u8* data, u64 size);

        renderer::VertexInputAttribDescription m_description;

        stream::Stream* m_indexStreamBuffer;
        stream::Stream* m_vertexStreamBuffer;

        u32 m_indexCount;
        BufferData m_indexData;

        u32 m_vertexCount;
        BufferData m_vertexData;

        std::vector<SubMesh*> m_subMeshes;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
