#pragma once

#include "Common.h"

namespace v3d
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
        TextureCubeMapArray,
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

    enum TextureSamples : s16
    {
        SampleCount_x1 = 0x1,
        SampleCount_x2 = 0x2,
        SampleCount_x4 = 0x4,
        SampleCount_x8 = 0x8,
        SampleCount_x16 = 0x16,
        SampleCount_x32 = 0x32,
        SampleCount_x64 = 0x64,

        RenderTargetSamples_Count
    };

    enum class RenderTargetLoadOp : s16
    {
        LoadOp_Load,
        LoadOp_Clear,
        LoadOp_DontCare
    };

    enum class RenderTargetStoreOp : s16
    {
        StoreOp_Store,
        StoreOp_DontCare
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct AttachmentDesc
    {
        renderer::ImageFormat _format;
        TextureSamples        _samples;
        RenderTargetLoadOp    _loadOp;
        RenderTargetStoreOp   _storeOp;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
