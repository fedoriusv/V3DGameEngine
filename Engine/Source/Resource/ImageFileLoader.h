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
    };

    typedef u32 ImageLoaderFlags;

    /**
    * ImageFileLoader class. Loader from file
    */
    class ImageFileLoader : public ResourceLoader<resource::Image*>
    {
    public:

        ImageFileLoader(u32 flags) noexcept;
        ~ImageFileLoader();

        resource::Image* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
