#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace scene
{
    class Model;
} //namespace scene

namespace resource
{
    struct ResourceHeader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum ModelLoaderFlag : u32
    {
        ModelLoaderFlag_SeperateMesh = 1 << 0,
        ModelLoaderFlag_SplitLargeMeshes = 1 << 1,
        ModelLoaderFlag_NoGenerateIndex = 1 << 2,
        //ModelLoaderFlag_GenerateTextureCoord = 1 << 3,
        ModelLoaderFlag_GenerateNormals = 1 << 4,
        ModelLoaderFlag_GenerateTangent = 1 << 5,
        ModelLoaderFlag_ReadHeader = 1 << 6,
        ModelLoaderFlag_LocalTransform = 1 << 7,
        ModelLoaderFlag_FlipYPosition = 1 << 8,
    };
    typedef u32 ModelLoaderFlags;

    /**
    * ModelFileLoader class. Loader from file
    */
    class ModelFileLoader : public ResourceLoader<scene::Model*>
    {
    public:

        ModelFileLoader(u32 flags) noexcept;
        ModelFileLoader(const ResourceHeader* header, u32 flags) noexcept;
        ~ModelFileLoader();

        scene::Model* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
