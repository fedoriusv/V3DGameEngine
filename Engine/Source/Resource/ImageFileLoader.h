#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace resource
{
    class Image;
} //namespace scene

namespace resource
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum ImageLoaderFlag : u32
    {
        ImageLoaderFlag_FlipY = 1 << 0,
        ImageLoaderFlag_GenerateMipmaps = 1 << 1,
    };

    typedef u32 ImageLoaderFlags;

    /**
    * @brief ImageFileLoader class. Loader from file
    */
    class ImageFileLoader : public ResourceLoader<resource::Image*>
    {
    public:

        explicit ImageFileLoader(u32 flags) noexcept;
        ~ImageFileLoader() = default;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        resource::Image* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
