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
