#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Stream/Stream.h"
#include "Renderer/Render.h"
#include "Renderer/PipelineState.h"

namespace v3d
{
namespace scene
{
    class MeshHelper;
} //namespace scene
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief maxTextureCoordsCount.
    * Num of UV channels
    */
    constexpr u32 k_maxTextureCoordsCount = 4;

    /**
    * @brief maxVertexColorIndex.
    * Num of Color channels
    */
    constexpr u32 k_maxVertexColorCount = 4;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief MeshResource
    */
    class MeshResource : public resource::Resource
    {
    public:

        struct MeshHeader : resource::ResourceHeader
        {
            MeshHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Mesh)
            {
            }

            ~MeshHeader() noexcept = default;
        };
        
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

        MeshResource() noexcept;
        explicit MeshResource(const MeshHeader& header) noexcept;
        ~MeshResource();

        u32 getVertexBufferCount() const;
        u64 getVertexBufferSize(u32 stream = 0) const;
        const void* getVertexBufferData(u32 stream = 0) const;

        u32 getIndexBufferCount() const;
        u64 getIndexBufferSize() const;
        const void* getIndexBufferData() const;
        renderer::IndexBufferType getIndexBufferType() const;

        const renderer::VertexInputAttributeDesc& getInputAttributeDesc() const;

    protected:

        friend scene::MeshHelper;

        MeshResource(const MeshResource&) = delete;
        MeshResource& operator=(const MeshResource&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        struct IndexData
        {
            std::tuple<stream::Stream*, void*>  _indexBuffer = { nullptr, 0 };
            u32                                 _indexCount  = 0;
            renderer::IndexBufferType           _indexType   = renderer::IndexBufferType::IndexType_16;
        };

        struct VertexData
        {
            std::vector<std::tuple<stream::Stream*, void*>> _vertexBuffers;
            u32                                             _vertexCount = 0;
        };


        MeshHeader                          m_header;
        renderer::VertexInputAttributeDesc  m_description;
        IndexData                           m_indexData;
        VertexData                          m_vertexData;

    };

    inline const renderer::VertexInputAttributeDesc& MeshResource::getInputAttributeDesc() const
    {
        return m_description;
    }

    inline u32 MeshResource::getVertexBufferCount() const
    {
        return m_vertexData._vertexCount;
    }

    inline u64 MeshResource::getVertexBufferSize(u32 stream) const
    {
        ASSERT(stream < m_vertexData._vertexBuffers.size(), "range out");
        return std::get<0>(m_vertexData._vertexBuffers[stream])->size();
    }

    inline const void* MeshResource::getVertexBufferData(u32 stream) const
    {
        ASSERT(stream < m_vertexData._vertexBuffers.size(), "range out");
        ASSERT(std::get<0>(m_vertexData._vertexBuffers[stream])->isMapped(), "not mapped");
        return std::get<1>(m_vertexData._vertexBuffers[stream]);
    }

    inline u32 MeshResource::getIndexBufferCount() const
    {
        return m_indexData._indexCount;
    }

    inline u64 MeshResource::getIndexBufferSize() const
    {
        ASSERT(std::get<0>(m_indexData._indexBuffer), "must be valid");
        return std::get<0>(m_indexData._indexBuffer)->size();
    }

    inline const void* MeshResource::getIndexBufferData() const
    {
        ASSERT(std::get<0>(m_indexData._indexBuffer), "must be valid");
        ASSERT(std::get<0>(m_indexData._indexBuffer)->isMapped(), "not mapped");
        return std::get<1>(m_indexData._indexBuffer);
    }

    inline renderer::IndexBufferType MeshResource::getIndexBufferType() const
    {
        return m_indexData._indexType;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
