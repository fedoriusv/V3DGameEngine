#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"
#include "Scene/Material.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace scene
namespace scene
{
    class Model;
} //namespace scene
namespace resource
{
    struct ResourceHeader;
    class ModelResource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ModelFileLoader class. Loader from file
    * 
    * @see MeshAssimpDecoder
    */
    class ModelFileLoader : public ResourceLoader<scene::Model*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ModelLoaderFlag enum
        */
        enum ModelLoaderFlag : u32
        {
            SkipIndexBuffer         = 1 << 0,   //Don't create Index Buffer
            SkipMaterial            = 1 << 1,
            SkipLights              = 1 << 2,
            SkipCameras             = 1 << 3,

            SeperatePositionStream  = 1 << 4,   //Save Position data to saparate stream

            FlipYPosition           = 1 << 5,   //Flip Y position
            FlipYTextureCoord       = 1 << 6,   //Flip Y texture coordinage

            LocalTransform          = 1 << 7,   //Ignore all releative transforms
            Optimization            = 1 << 8,
            OverridedShadingModel   = 1 << 9

        };
        typedef u32 ModelLoaderFlags;

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

        struct ModelPolicy : ResourceDecoder::Policy
        {
            VertexProperiesFlags        vertexProperies = 0;
            f32                         scaleFactor = 1.f;
            scene::MaterialShadingModel overridedShadingModel = scene::MaterialShadingModel::Custom;
        };
        using PolicyType = ModelPolicy;

        /**
        * @brief ModelFileLoader constructor
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        explicit ModelFileLoader(renderer::Device* device, ModelLoaderFlags flags) noexcept;

        /**
        * @brief ModelFileLoader constructor. Create a Model by Header
        * @param const ResourceHeader* header [required]
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        explicit ModelFileLoader(renderer::Device* device, const ModelFileLoader::ModelPolicy& policy, ModelLoaderFlags flags) noexcept;

        /**
        * @brief ModelFileLoader destructor
        */
        ~ModelFileLoader() = default;

        /**
        * @brief Load model resource by name from file
        * @see Model
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Model pointer
        */
        [[nodiscard]] scene::Model* load(const std::string& name, const std::string& alias = "") override;

    private:

        ModelFileLoader() = delete;
        ModelFileLoader(const ModelFileLoader&) = delete;
        ModelFileLoader& operator=(const ModelFileLoader&) = delete;

        ModelPolicy      m_policy;
        ModelLoaderFlags m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
