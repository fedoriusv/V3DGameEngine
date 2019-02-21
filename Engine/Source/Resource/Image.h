#pragma once

#include "Common.h"
#include "Resource.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ImageHeader struct.
    */
    struct ImageHeader : ResourceHeader
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Image. Resource class
    */
    class Image : public Resource
    {
        Image() noexcept;
        ~Image();

        void init(stream::Stream* stream) override;
        bool load() override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
