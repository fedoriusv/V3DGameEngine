#pragma once

#include "Common.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Image base class. Render side
    */
    class Image : public utils::Observable
    {
    public:

        Image() {};
        virtual ~Image() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void clear(Context* context, const core::Vector4D& color) = 0;
        virtual void clear(Context* context, f32 depth, u32 stencil) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
