#pragma once

#include "Common.h"
#include "ImageFormats.h"
#include "Utils/Observable.h"

namespace v3d
{

    class Object;

namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    class Image : public utils::Observable
    {
    public:

        Image() {};
        virtual ~Image() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void clear(const Context* context, const core::Vector4D& color) = 0;
        virtual void clear(const Context* context, f32 depth, u32 stencil) = 0;

    private:

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
