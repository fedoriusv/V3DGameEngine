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
    * @brief ModelLoaderFlag enum
    */
    enum ModelLoaderFlag : u32
    {
        ModelLoaderFlag_SeperateMesh = 1 << 0,
        ModelLoaderFlag_SplitLargeMeshes = 1 << 1,
        ModelLoaderFlag_NoGenerateIndex = 1 << 2,
        //ModelLoaderFlag_GenerateTextureCoord = 1 << 3,
        ModelLoaderFlag_GenerateNormals = 1 << 4,
        ModelLoaderFlag_GenerateTangentAndBitangent = 1 << 6,
        ModelLoaderFlag_UseBitangent = 1 << 7,
        ModelLoaderFlag_ReadHeader = 1 << 8,
        ModelLoaderFlag_LocalTransform = 1 << 9,
        ModelLoaderFlag_FlipYPosition = 1 << 10,
        ModelLoaderFlag_FlipYTextureCoord = 1 << 11,
    };
    typedef u32 ModelLoaderFlags;

    /**
    * @brief ModelFileLoader class. Loader from file
    * @see MeshAssimpDecoder
    */
    class ModelFileLoader : public ResourceLoader<scene::Model*>, public ResourceDecoderRegistration
    {
    public:

        ModelFileLoader() = delete;
        ModelFileLoader(const ModelFileLoader&) = delete;
        ~ModelFileLoader() = default;

        /**
        * @brief ModelFileLoader constructor
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        ModelFileLoader(ModelLoaderFlags flags) noexcept;

        /**
        * @brief ModelFileLoader constructor. Create a Model by Header
        * @param const ResourceHeader* header [required]
        * @param ModelLoaderFlags flags [required]
        * @see ModelLoaderFlags
        */
        ModelFileLoader(const ResourceHeader* header, ModelLoaderFlags flags) noexcept;

        /**
        * @brief Load model resource by name from file
        * @see Model
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Model pointer
        */
        scene::Model* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
