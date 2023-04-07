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

        /**
        * @brief Subresource struct. Render side.
        * Size 8 bytes
        */
        struct Subresource
        {
            bool operator==(const Subresource& sub)
            {
                return _baseLayer == sub._baseLayer && _layers == sub._layers && _baseMip == sub._baseMip && _mips == sub._mips;
            }

            u32 _baseLayer  : 16;
            u32 _layers     : 16;
            u32 _baseMip    : 16;
            u32 _mips       : 16;
        };

        [[nodiscard]] static const Subresource makeImageSubresource(u32 baseLayer, u32 layers, u32 baseMip, u32 mips)
        {
            static_assert(sizeof(Subresource) == 8, "Wrong size");
            ASSERT(baseLayer != ~0 && baseMip != ~0 && layers != ~0 && mips != ~0, "must be real");
            return { baseLayer, layers, baseMip, mips };
        }

        Image() = default;
        virtual ~Image() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void clear(Context* context, const math::Vector4D& color) = 0;
        virtual void clear(Context* context, f32 depth, u32 stencil) = 0;

        virtual bool upload(Context* context, const math::Dimension3D& size, u32 layers, u32 mips, const void* data) = 0;
        virtual bool upload(Context* context, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, const void* data) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
