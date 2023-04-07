#pragma once

#include "Common.h"
#include "Scene/Component.h"
#include "Scene/Material.h"
#include "Resource/Resource.h"
#include "Mesh.h"

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
        enum ModelContent : u16
        {
            ModelContent_Empty = 0,
            ModelContent_Mesh = 1 << 0,
            ModelContent_Material = 1 << 1,
            ModelContent_LODs = 1 << 2,
        };
        typedef u16 ModelContentFlags;

        ModelHeader() noexcept;
        ~ModelHeader() = default;

        u32 operator>>(stream::Stream* stream) override;
        u32 operator<<(const stream::Stream* stream) override;

        u32 _numMeshes;
        u32 _numMaterials;
        ModelContentFlags _modelContentFlags;
        bool _localTransform;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class. Resource
    */
    class Model : public resource::Resource
    {
    public:

        Model() noexcept;
        explicit Model(const ModelHeader* header) noexcept;
        ~Model() noexcept;

        std::vector<Mesh*> getMeshes(u32 LOD = 0) const;
        Mesh* getMeshByIndex(u32 index, u32 LOD = 0) const;
        u32   getMeshCount() const;

        std::vector<Material*> getMaterials() const;
        Material* getMaterialByIndex(u32 index) const;
        u32 getMaterialsCount() const;

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        const ModelHeader& getModelHeader() const;

        std::vector<std::vector<Mesh*>> m_meshes;
        std::vector<Material*> m_materials;

        friend class ModelHelper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
