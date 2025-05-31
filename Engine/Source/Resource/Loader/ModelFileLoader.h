#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

namespace v3d
{
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
    class ModelFileLoader : public ResourceLoader<ModelResource*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ModelLoaderFlag enum
        */
        enum ModelLoaderFlag : u32
        {
            SkipIndexBuffer = 1 << 0,                    //Don't create Index Buffer
            SkipNormals = 1 << 1,                        //Don't use Normal attribute
            SkipTangentAndBitangent = 1 << 2,            //Don't use Tangent & Bitangetns attributes
            SkipTextureCoord = 1 << 3,                   //Don't use Texture attributes
            SkipMaterial = 1 << 4,

            SeperatePositionStream = 1 << 5,             //Save Position data to saparate stream
            GenerateBoundingBoxes = 1 << 6,              //Generate BoundingBox for meshes

            FlipYPosition = 1 << 7,                      //Flip Y position
            FlipYTextureCoord = 1 << 8,                  //Flip Y texture coordinage

            LocalTransform = 1 << 9,                     //Ignore all releative transforms
            SplitLargeMeshes = 1 << 10,

        };
        typedef u32 ModelLoaderFlags;

        struct ModelPolicy : ResourceDecoder::Policy
        {
        };

        /**
        * @brief ModelFileLoader constructor
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        explicit ModelFileLoader(ModelLoaderFlags flags) noexcept;

        /**
        * @brief ModelFileLoader constructor. Create a Model by Header
        * @param const ResourceHeader* header [required]
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        explicit ModelFileLoader(const ModelFileLoader::ModelPolicy& policy, ModelLoaderFlags flags) noexcept;

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
        [[nodiscard]] ModelResource* load(const std::string& name, const std::string& alias = "") override;

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
