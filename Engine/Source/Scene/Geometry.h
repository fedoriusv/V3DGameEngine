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
    * @brief GeometryHeader meta info of Geometry
    */
    struct GeometryHeader : resource::ResourceHeader
    {
        GeometryHeader() noexcept;

        struct GeometryInfo
        {
            GeometryInfo();

            struct SubData
            {
                u64 _offset;
                u64 _size;
                u32 _count;
            };

            u32                  _count;
            u64                  _size;
            std::vector<SubData> _subData;

            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        GeometryInfo    _index;
        GeometryInfo    _vertex;

        u64             _size;
        u64             _offset;

        enum GeometryFlag
        {
            GeometryFlag_PresentIndex = 1 << 0,
        };

        u16 _flags;
    };

    /**
    * @brief Geometry class. Contains mesh geometry.
    */
    class Geometry : public resource::Resource
    {
    public:

        explicit Geometry(GeometryHeader* header) noexcept;
        ~Geometry();

        void init(stream::Stream* stream) override;
        bool load() override;

        const renderer::VertexInputAttribDescription& getVertexInputAttribDesc() const;

        u8* getVertexData(s32 subGeometry = -1) const;
        u32 getVertexCount(s32 subGeometry = -1) const;
        u64 getVertexSize(s32 subGeometry = -1) const;

        u8* getIndexData(s32 subGeometry = -1) const;
        u32 getIndexCount(s32 subGeometry = -1) const;
        u64 getIndexSize(s32 subGeometry = -1) const;

    private:

        struct BufferData
        {
            u8* _data;
            u64 _size;
        };

        class SubGeometry final
        {
        public:

            SubGeometry() noexcept;
            ~SubGeometry() = default;

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

            friend Geometry;
        };

        const GeometryHeader& getGeometryHeader() const;
        void fillVertexData(u32 count, u8* data, u64 size);
        void fillIndexData(u32 count, u8* data, u64 size);

        renderer::VertexInputAttribDescription m_description;

        stream::Stream* m_indexStreamBuffer;
        stream::Stream* m_vertexStreamBuffer;

        u32 m_indexCount;
        BufferData m_indexData;

        u32 m_vertexCount;
        BufferData m_vertexData;

        std::vector<SubGeometry*> m_subGeometry;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
