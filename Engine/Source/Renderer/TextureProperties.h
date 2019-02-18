#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * TextureUsage enum. usageFlag inside Texture
    */
    enum TextureUsage : u16
    {
        TextureUsage_Write = 0x01,
        TextureUsage_Read = 0x02,

        TextureUsage_Sampled = 0x04,
        TextureUsage_Attachment = 0x08,

        TextureUsage_Shared = 0x64,
    };

    typedef u16 TextureUsageFlags;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class TextureTarget : u16
    {
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCubeMap,
    };

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

    enum class TextureSamples : u32
    {
        TextureSamples_x1 = 0,
        TextureSamples_x2 = 1,
        TextureSamples_x4 = 2,
        TextureSamples_x8 = 3,
        TextureSamples_x16 = 4,
        TextureSamples_x32 = 5,
        TextureSamples_x64 = 6,
    };

    enum class RenderTargetLoadOp : u32
    {
        LoadOp_DontCare,
        LoadOp_Clear,
        LoadOp_Load,
    };

    enum class RenderTargetStoreOp : u32
    {
        StoreOp_DontCare,
        StoreOp_Store,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxFramebufferAttachments = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * AttachmentDescription struct
    * 4 byte size
    */
    struct AttachmentDescription
    {
        AttachmentDescription()
        {
            memset(this, 0, sizeof(AttachmentDescription));
        }

        Format                _format           : 8;
        RenderTargetLoadOp    _loadOp           : 2;
        RenderTargetStoreOp   _storeOp          : 2;
        RenderTargetLoadOp    _stencilLoadOp    : 2;
        RenderTargetStoreOp   _stencilStoreOp   : 2;
        TextureSamples        _samples          : 3;
        u32                  _internalTarget    : 1;

        u32                   _padding          : 12;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
