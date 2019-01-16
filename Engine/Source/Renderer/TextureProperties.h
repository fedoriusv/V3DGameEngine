#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class TextureTarget : s16
    {
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCubeMap,
    };

    enum TextureFilter : s16
    {
        TextureFilter_Nearest = 0x1,
        TextureFilter_Linear = 0x2,

        TextureFilter_NearestMipmapNearest = 0x4,
        TextureFilter_NearestMipmapLinear = 0x8,
        TextureFilter_LinearMipmapNearest = 0x16,
        TextureFilter_LinearMipmapLinear = 0x32
    };

    enum class TextureAnisotropic : s16
    {
        TextureAnisotropic_None = 0,
        TextureAnisotropic_2x = 1 << 1,
        TextureAnisotropic_4x = 1 << 2,
        TextureAnisotropic_8x = 1 << 3,
        TextureAnisotropic_16x = 1 << 4,
    };

    enum class TextureWrap : s16
    {
        TextureWrap_Repeat,
        TextureWrap_MirroredRepeat,
        TextureWrap_ClampToEdge,
        TextureWrap_ClampToBorder,
    };

    enum class TextureSamples : s32
    {
        TextureSamples_x1 = 0x1,
        TextureSamples_x2 = 0x2,
        TextureSamples_x4 = 0x4,
        TextureSamples_x8 = 0x8,
        TextureSamples_x16 = 0x16,
        TextureSamples_x32 = 0x32,
        TextureSamples_x64 = 0x64,
    };

    enum class RenderTargetLoadOp : s32
    {
        LoadOp_DontCare,
        LoadOp_Clear,
        LoadOp_Load,
    };

    enum class RenderTargetStoreOp : s32
    {
        StoreOp_DontCare,
        StoreOp_Store,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxFramebufferAttachments = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * AttachmentDescription
    */
    struct AttachmentDescription
    {
        AttachmentDescription()
        {
            memset(this, 0, sizeof(AttachmentDescription));
            _samples = TextureSamples::TextureSamples_x1;
        }


        Format                _format           : 8;
        TextureSamples        _samples          : 4;
        RenderTargetLoadOp    _loadOp           : 2;
        RenderTargetStoreOp   _storeOp          : 2;
        RenderTargetLoadOp    _stencilLoadOp    : 2;
        RenderTargetStoreOp   _stencilStoreOp   : 2;

        s32                   _padding          : 12;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
