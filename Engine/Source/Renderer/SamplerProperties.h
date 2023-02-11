#pragma once

#include "Common.h"
#include "Formats.h"
#include "PipelineStateProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerFilter enum
    */
    enum SamplerFilter : u32
    {
        SamplerFilter_Nearest = 0, //min, mag, mipmaps - no filtering
        SamplerFilter_Bilinear,    //min, mag - linear filtering, mipmaps - no filtering
        SamplerFilter_Trilinear,   //min, mag, mipmaps - linear filtering
    };

    /**
    * @brief SamplerAnisotropic enum class
    */
    enum class SamplerAnisotropic : u32
    {
        SamplerAnisotropic_None = 0,
        SamplerAnisotropic_2x = 1 << 1,
        SamplerAnisotropic_4x = 1 << 2,
        SamplerAnisotropic_8x = 1 << 3,
        SamplerAnisotropic_16x = 1 << 4,
    };

    /**
    * @brief SamplerWrap enum class
    */
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
    * @brief SamplerDescription. The struct describes sampler parameters
    */
    struct SamplerDescription
    {
        SamplerDescription() noexcept;
        bool operator==(const SamplerDescription& other) const;

        /**
        * @brief SamplerDesc struct. Size 24 bytes
        */
        struct SamplerDesc
        {
            core::Vector4D          _borderColor;
            f32                     _lodBias;

            SamplerAnisotropic      _anisotropic  : 5;
            SamplerWrap             _wrapU        : 3;
            SamplerWrap             _wrapV        : 3;
            SamplerWrap             _wrapW        : 3;
            SamplerFilter           _filter       : 2;
            CompareOperation        _compareOp    : 3;
            u32                     _enableCompOp : 1;

            u32                     _padding      : 12;
        };

        SamplerDesc                 _desc;

        struct Hash
        {
            u32 operator()(const SamplerDescription& desc) const;
        };

        struct Compare
        {
            bool operator()(const SamplerDescription& op1, const SamplerDescription& op2) const;
        };

        void dirty();

    private:

        mutable u32 _hash;
        mutable bool _dirty;

        void recalculateHash() const;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
