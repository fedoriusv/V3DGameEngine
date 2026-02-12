#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Stream/Stream.h"
#include "Renderer/PipelineState.h"
#include "Scene/SceneNode.h"
#include "Scene/Material.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace renderer
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

    class Mesh;
    class Light;
    class Camera;
    class Material;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class. Resource
    */
    class Model final : /*private*/public SceneNode,/* public Object,*/ public resource::Resource
    {
    public:

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

        /**
        * @brief LoadPolicy
        */
        struct LoadPolicy : resource::Resource::LoadPolicy
        {
            VertexProperiesFlags        vertexProperies = VertexProperies_Empty;
            f32                         scaleFactor = 1.f;
            scene::MaterialShadingModel overridedShadingModel = scene::MaterialShadingModel::Custom;
            bool                        unique = false;
        };

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

        explicit Model(renderer::Device* device) noexcept;
        explicit Model(renderer::Device* device, const ModelHeader& header) noexcept;

    private:

        Model(const Model& model) noexcept;
        ~Model() noexcept;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        bool loadNode(SceneNode* node, const stream::Stream* stream, u32 offset);

        ModelHeader                     m_header;
        renderer::Device* const         m_device;

        std::vector<Mesh*>              m_meshes;
        std::vector<Material*>          m_materials;
        std::vector<Light*>             m_lights;
        std::vector<Camera*>            m_cameras;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Model>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace v3d
