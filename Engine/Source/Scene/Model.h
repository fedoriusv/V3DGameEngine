#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Stream/Stream.h"
#include "Renderer/PipelineState.h"
#include "Scene/Scene.h"

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
    class Material;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class. Resource
    */
    class Model final : /*private*/public SceneNode, public Object, public resource::Resource
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

        explicit Model(renderer::Device* device) noexcept;
        explicit Model(renderer::Device* device, const ModelHeader& header) noexcept;
        ~Model() noexcept;

    private:

        Model(const Model& model) noexcept;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        bool loadNode(SceneNode* node, const stream::Stream* stream, u32 offset);

        ModelHeader                     m_header;
        renderer::Device* const         m_device;

        std::vector<Mesh*>              m_meshes;
        std::vector<Material*>          m_materials;
        std::vector<Light*>             m_lights;
        std::vector<Camera*>            m_cameras;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
