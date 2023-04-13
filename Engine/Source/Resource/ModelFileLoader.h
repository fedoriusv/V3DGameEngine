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
            ReadHeader = 1 << 0,

            SkipIndexBuffer = 1 << 1,                    //Don't create Index Buffer
            SkipNormals = 1 << 2,                        //Don't use Normal attribute
            SkipTangentAndBitangent = 1 << 3,            //Don't use Tangent & Bitangetns attributes
            SkipTextureCoordt = 1 << 4,                  //Don't use Texture attributes

            SeperatePosition = 1 << 5,                   //Save Position to saparate stream
            UseBoundingBoxes = 1 << 6,                   //Generate BoundingBox for meshes

            LocalTransform = 1 << 7,                     //Ignore all releative transforms
            FlipYPosition = 1 << 8,                      //Flip Y position
            FlipYTextureCoord = 1 << 9,                  //Flip Y texture coordinage

            SplitLargeMeshes = 1 << 10,
            SkipMaterialLoading = 1 << 11,

        };
        typedef u32 ModelLoaderFlags;

        ModelFileLoader() = delete;
        ModelFileLoader(const ModelFileLoader&) = delete;
        ModelFileLoader& operator=(const ModelFileLoader&) = delete;

        /**
        * @brief ModelFileLoader constructor
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        explicit ModelFileLoader(ModelLoaderFlags flags) noexcept;
        ~ModelFileLoader() = default;

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
