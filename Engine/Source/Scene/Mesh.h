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
    * Mesh class. Component, Resource
    */
    class Mesh : public Component, public resource::Resource
    {
    public:

        Mesh() noexcept;
        ~Mesh();

        void init(stream::Stream* stream) override;
        bool load() override;

        const renderer::VertexInputAttribDescription& getVertexInputAttribDesc() const;

    private:

        class SubMesh final
        {
        public:

            SubMesh() noexcept;
            ~SubMesh();

            u8* getVertexData() const;
            u8* getIndexData() const;

        private:

            void fillVertexData(u32 count, u8* data, u64 size);
            void fillIndexData(u32 count, u8* data, u64 size);

            struct BufferData
            {
                u8* _data;
                u64 _size;
            };

            u32 m_indexCount;
            BufferData m_indexData;

            u32 m_vertexCount;
            BufferData m_vertexData;

            friend Mesh;
        };

        renderer::VertexInputAttribDescription m_description;

        stream::Stream* m_indexStreamBuffer;
        stream::Stream* m_vertexStreamBuffer;

        std::vector<SubMesh*> m_subMeshes;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
