#pragma once

#include "Common.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class MeshResource;

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
            ModelContent_AABB = 1 << 2,
        };
        typedef u32 ModelContentFlags;

    private:

        ModelResource(const ModelResource&) = delete;
        ModelResource& operator=(const ModelResource&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        struct Geometry
        {
            std::vector<MeshResource*> _LODs;
        };

        ModelHeader             m_header;
        std::vector<Geometry>   m_geometry;
        ModelContentFlags       m_content;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
