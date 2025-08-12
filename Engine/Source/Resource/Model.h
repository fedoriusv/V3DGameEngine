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
    class ModelHelper;
    class MeshHelper;
} //namespace scene
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class Bitmap;

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
    * @brief ModelResource class. Resource
    */
    class ModelResource : public resource::Resource
    {
    public:

        /**
        * @brief ModelHeader meta info about Model
        */
        struct ModelHeader : resource::ResourceHeader
        {
            ModelHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Model)
            {
            }
        };

        ModelResource() noexcept;
        explicit ModelResource(const ModelHeader& header) noexcept;
        ~ModelResource() noexcept;

        /**
        * @brief ModelContent enum
        */
        enum ModelContent : u32
        {
            ModelContent_Empty = 0,
            ModelContent_Meshes = 1 << 0,
            ModelContent_Materials = 1 << 1,
            ModelContent_Lights = 1 << 2,
        };
        typedef u32 ModelContentFlags;

        /**
        * @brief MeshResource struct
        */
        struct MeshResource
        {
            struct IndexData
            {
                std::tuple<stream::Stream*, void*>              _indexBuffer = { nullptr, 0 };
                u32                                             _indexCount = 0;
                renderer::IndexBufferType                       _indexType = renderer::IndexBufferType::IndexType_16;
            };

            struct VertexData
            {
                std::vector<std::tuple<stream::Stream*, void*>> _vertexBuffers;
                u32                                             _vertexCount = 0;
            };

            std::string                         m_name;
            renderer::VertexInputAttributeDesc  m_description;
            IndexData                           m_indexData;
            VertexData                          m_vertexData;
        };

        /**
        * @brief MaterialResource struct
        */
        struct MaterialResource
        {
            using Property = std::variant<std::monostate, f32, math::float4, u32>;

            std::string m_name;
            std::vector<std::tuple<std::string, Property>> m_properties;
        };

    private:

        friend scene::ModelHelper;
        friend scene::MeshHelper;

        ModelResource(const ModelResource&) = delete;
        ModelResource& operator=(const ModelResource&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        MeshResource* loadMesh(const stream::Stream* stream, u32 offset);
        MaterialResource* loadMaterial(const stream::Stream* stream, u32 offset);

        struct Geometry
        {
            std::vector<MeshResource*> _LODs;
            std::vector<math::AABB>    _boundingBox;
        };

        ModelHeader                     m_header;
        std::vector<Geometry>           m_geometry;
        std::vector<MaterialResource*>  m_materials;
        std::vector<Bitmap*>            m_images;
        ModelContentFlags               m_contentFlags;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
