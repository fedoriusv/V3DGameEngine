#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"

namespace v3d
{
namespace scene
{
    class Model;
} //namespace scene
namespace resource
{
    struct ResourceHeader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ModelFileLoader class. Loader from file
    * @see MeshAssimpDecoder
    */
    class ModelFileLoader : public ResourceLoader<scene::Model*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ModelLoaderFlag enum
        */
        enum ModelLoaderFlag : u16
        {
            ReadHeader = 1 << 1,

            SkipIndexBuffer = 1 << 2,                    //Don't create Index Buffer
            SkipNormalsAttribute = 1 << 3,               //Don't use Normal attribute
            SkipTangentAndBitangentAttribute = 1 << 4,   //Don't use Tangent & Bitangetns attributes
            SkipTextureCoordtAttributes = 1 << 5,        //Don't use Texture attributes

            SeperatePositionAttribute = 1 << 6,          //Save Position to saparate stream
            UseBoundingBoxes = 1 << 7,                   //Generate BoundingBox for meshes

            LocalTransform = 1 << 8,                     //Ignore all releative transforms
            FlipYPosition = 1 << 9,                      //Flip Y position
            FlipYTextureCoord = 1 << 10,                 //Flip Y texture coordinage

            SplitLargeMeshes = 1 << 11,
            SkipMaterialLoading = 1 << 12,

        };
        typedef u16 ModelLoaderFlags;

        ModelFileLoader() = delete;
        ModelFileLoader(const ModelFileLoader&) = delete;
        ~ModelFileLoader() = default;

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
        explicit ModelFileLoader(const ResourceHeader* header, ModelLoaderFlags flags) noexcept;

        /**
        * @brief Load model resource by name from file
        * @see Model
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Model pointer
        */
        [[nodiscard]] scene::Model* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
