#pragma once

#include "Common.h"
#include "ImageFormats.h"

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
        TextureFilterNearest = 0x1,
        TextureFilterLinear = 0x2,

        TextureFilterNearestMipmapNearest = 0x4,
        TextureFilterNearestMipmapLinear = 0x8,
        TextureFilterLinearMipmapNearest = 0x16,
        TextureFilterLinearMipmapLinear = 0x32
    };

    enum class TextureAnisotropic : s16
    {
        TextureAnisotropicNone = 0,
        TextureAnisotropic2x = 1 << 1,
        TextureAnisotropic4x = 1 << 2,
        TextureAnisotropic8x = 1 << 3,
        TextureAnisotropic16x = 1 << 4,
    };

    enum class TextureWrap : s16
    {
        TextureRepeat,
        TextureMirroredRepeat,
        TextureClampToEdge,
        TextureClampToBorder,
    };

    enum class TextureSamples : s32
    {
        SampleCount_x1 = 0x1,
        SampleCount_x2 = 0x2,
        SampleCount_x4 = 0x4,
        SampleCount_x8 = 0x8,
        SampleCount_x16 = 0x16,
        SampleCount_x32 = 0x32,
        SampleCount_x64 = 0x64,
    };

    enum class RenderTargetLoadOp : s32
    {
        LoadOp_Load,
        LoadOp_Clear,
        LoadOp_DontCare
    };

    enum class RenderTargetStoreOp : s32
    {
        StoreOp_Store,
        StoreOp_DontCare
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxFramebufferAttachments = 8;

    struct AttachmentDescription
    {
        ImageFormat           _format           : 8;
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
