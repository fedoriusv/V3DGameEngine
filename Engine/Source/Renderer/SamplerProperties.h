#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum SamplerFilter : s16
    {
        SamplerFilter_Nearest = 0x1,
        SamplerFilter_Bilinear = 0x2,
        SamplerFilter_Trilinear = 0x4,
        SamplerFilter_Cubic = 0x8
    };

    enum class SamplerAnisotropic : s16
    {
        SamplerAnisotropic_None = 0,
        SamplerAnisotropic_2x = 1 << 1,
        SamplerAnisotropic_4x = 1 << 2,
        SamplerAnisotropic_8x = 1 << 3,
        SamplerAnisotropic_16x = 1 << 4,
    };

    enum class SamplerWrap : s16
    {
        TextureWrap_Repeat,
        TextureWrap_MirroredRepeat,
        TextureWrap_ClampToEdge,
        TextureWrap_ClampToBorder,
        TextureWrap_MirroredClampToEdge,
    };

    /**
    * SamplerDescription
    */
    struct SamplerDescription
    {
        SamplerFilter       _magFilter;
        SamplerFilter       _minFilter;
        SamplerAnisotropic  _anisotropic;
        SamplerWrap         _wrapU;
        SamplerWrap         _wrapW;
        SamplerWrap         _wrapR;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
