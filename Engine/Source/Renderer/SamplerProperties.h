#pragma once

#include "Common.h"
#include "Formats.h"
#include "PipelineStateProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum SamplerFilter : u32
    {
        SamplerFilter_Nearest = 0,
        SamplerFilter_Bilinear,
        SamplerFilter_Trilinear,
        SamplerFilter_Cubic
    };

    enum class SamplerAnisotropic : u32
    {
        SamplerAnisotropic_None = 0,
        SamplerAnisotropic_2x = 1 << 1,
        SamplerAnisotropic_4x = 1 << 2,
        SamplerAnisotropic_8x = 1 << 3,
        SamplerAnisotropic_16x = 1 << 4,
    };

    enum class SamplerWrap : u32
    {
        TextureWrap_Repeat = 0,
        TextureWrap_MirroredRepeat,
        TextureWrap_ClampToEdge,
        TextureWrap_ClampToBorder,
        TextureWrap_MirroredClampToEdge,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * SamplerDescription. struct
    */
    struct SamplerDescription
    {
        SamplerDescription() noexcept
        {
            memset(this, 0, sizeof(SamplerDescription));
        }

        SamplerAnisotropic _anisotropic  : 5;
        SamplerWrap        _wrapU        : 3;
        SamplerWrap        _wrapV        : 3;
        SamplerWrap        _wrapW        : 3;
        SamplerFilter      _magFilter    : 2;
        SamplerFilter      _minFilter    : 2;
        CompareOperation   _compareOp    : 3;
        bool               _enableCompOp : 1;

        u32                _padding      : 10;
                                         
        f32                _lodBias; //  : 32

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
