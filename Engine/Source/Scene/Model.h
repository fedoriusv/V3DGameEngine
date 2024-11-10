#pragma once

#include "Common.h"
#include "Scene/Component.h"
//#include "Scene/Material.h"
#include "Resource/Resource.h"
//#include "StaticMesh.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class. Component, Resource
    */
    class Model : public resource::Resource
    {
    public:

        /**
        * @brief ModelHeader meta info about Model
        */
        struct ModelHeader : resource::ResourceHeader
        {
            ModelHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::ModelResource)
            {
            }
        };

        Model() noexcept;
        explicit Model(const ModelHeader& header) noexcept;
        ~Model() noexcept;

        /**
        * @brief ModelContent enum
        */
        enum ModelContent : u32
        {
            ModelContent_Empty = 0,
            ModelContent_Mesh = 1 << 0,
            ModelContent_Material = 1 << 1,
            ModelContent_AABB = 1 << 2,
        };

        typedef u32 ModelContentFlags;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        ModelHeader m_header;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
