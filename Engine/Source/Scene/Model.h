#pragma once

#include "Common.h"
#include "Scene/Component.h"
#include "Scene/Material.h"
#include "Resource/Resource.h"
#include "StaticMesh.h"

namespace v3d
{
namespace scene
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
    * @brief ModelHeader meta info about Model
    */
    struct ModelHeader : resource::ResourceHeader
    {
        /**
        * @brief ModelContent enum
        */
        enum ModelContent : u32
        {
            ModelContent_Empty = 0,
            ModelContent_Mesh = 1 << 0,
            ModelContent_Material = 1 << 1,
        };
        typedef u32 ModelContentFlags;

        ModelHeader() noexcept;
        ModelHeader(const ModelHeader& other) noexcept;
        ~ModelHeader() = default;

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

        ModelContentFlags _modelContentFlags;
        u32 _localTransform;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class. Component, Resource
    */
    class Model : public resource::Resource
    {
    public:

        Model() noexcept;
        explicit Model(ModelHeader* header) noexcept;
        ~Model() noexcept;

        std::vector<Mesh*> getMesh(u32 index) const;
        Mesh* getMeshByIndex(u32 index, u32 LOD = 0) const;
        u32 getMeshCount() const;

        std::vector<Material*> getMaterials() const;
        Material* getMaterialByIndex(u32 index) const;
        u32 getMaterialsCount() const;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        ModelHeader* m_header;

        std::vector<std::vector<Mesh*>> m_meshes;
        std::vector<Material*> m_materials;

        friend class ModelHelper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
