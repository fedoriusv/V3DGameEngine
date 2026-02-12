#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"
#include "Scene/Material.h"
#include "Scene/Model.h"

namespace v3d
{
namespace renderer
{
    class Device;
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
    class ModelFileLoader : public ResourceLoader<scene::Model>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = scene::Model;
        using PolicyType = scene::Model::LoadPolicy;

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
            FlipYTextureCoord       = 1 << 6,   //Flip Y texture coordinate

            LocalTransform          = 1 << 7,   //Ignore all releative transforms
            Optimization            = 1 << 8,
            OverridedShadingModel   = 1 << 9

        };
        typedef u32 ModelLoaderFlags;

        /**
        * @brief ModelFileLoader constructor
        * @param renderer::Device* device [required]
        */
        explicit ModelFileLoader(renderer::Device* device) noexcept;

        /**
        * @brief ModelFileLoader destructor
        */
        ~ModelFileLoader() = default;

        /**
        * @brief Load model resource by name from file
        * @see Model
        * @param const std::string& name [required]
        * @param const PolicyType& policy [required]
        * @param ModelLoaderFlags flags [optional]
        * @return Model pointer
        */
        [[nodiscard]] scene::Model* load(const std::string& name, const Resource::LoadPolicy& policy, ModelLoaderFlags flags = 0) override;

    private:

        ModelFileLoader(const ModelFileLoader&) = delete;
        ModelFileLoader& operator=(const ModelFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
