#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/BufferProperties.h"
#include "Mesh.h"
#include "Material.h"

namespace v3d
{
namespace scene
{
    class Mesh;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ModleHeader meta info about Model
    */
    struct ModelHeader : resource::ResourceHeader
    {
        enum ModelContent : u32
        {
            ModelContext_Empty = 0,
            ModelContext_Mesh = 1 << 0,
            ModelContext_Material = 1 << 1
        };
        typedef u16 ModelContentFlags;

        enum VertexProperies : u32
        {
            VertexProperies_Empty = 0,
            VertexProperies_Position  = 1 << 0,
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

        ModelHeader() noexcept;
        ~ModelHeader();

        ModelContentFlags       _modelContentFlags;
        VertexProperiesFlags    _vertexContentFlags;

        renderer::PolygonMode   _mode;
        renderer::FrontFace     _frontFace;
        bool                    _localTransform;

        std::vector<MeshHeader>     _meshes;
        std::vector<MaterialHeader> _materials;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxTextureCoordsIndex = 4;
    constexpr u32 k_maxVertexColorIndex = 4;

    /**
    * Model class. Component, Resource
    */
    class Model : public Component, public resource::Resource
    {
    public:

        explicit Model(ModelHeader* header) noexcept;
        ~Model();

        const ModelHeader& getModelHeader() const;

        std::vector<Mesh*> getMeshes() const;
        Mesh* getMeshByIndex(u32 index) const;
        u32   getMeshCount() const;

        std::vector<Material*> getMaterials() const;
        Material* getMaterialByIndex(u32 index) const;
        u32 getMaterialsCount() const;

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        std::vector<Mesh*> m_meshes;
        std::vector<Material*> m_materials;

        friend class ModelHelper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d