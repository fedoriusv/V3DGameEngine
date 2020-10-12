#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Utils/ResourceID.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Image base class. Render side
    */
    class Image : public utils::Observable, public utils::ResourceID<Image, u64>
    {
    public:

        Image() = default;
        virtual ~Image() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void clear(Context* context, const core::Vector4D& color) = 0;
        virtual void clear(Context* context, f32 depth, u32 stencil) = 0;

        virtual bool upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data) = 0;
        virtual bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
