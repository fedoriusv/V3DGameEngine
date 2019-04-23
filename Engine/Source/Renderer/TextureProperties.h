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

        TextureUsage_Resolve = 0x10,

        TextureUsage_Shared = 0x40,
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

    enum class TransitionOp : u32
    {
        TransitionOp_Undefined,
        TransitionOp_ShaderRead,
        TransitionOp_ColorAttachmet,
        TransitionOp_DepthStencilAttachmet,

        TransitionOp_Present,
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
        AttachmentDescription() noexcept
        {
            memset(this, 0, sizeof(AttachmentDescription));
        }

        Format                _format           : 8;
        RenderTargetLoadOp    _loadOp           : 2;
        RenderTargetStoreOp   _storeOp          : 2;
        RenderTargetLoadOp    _stencilLoadOp    : 2;
        RenderTargetStoreOp   _stencilStoreOp   : 2;
        TextureSamples        _samples          : 3;
        u32                   _internalTarget   : 1;

        TransitionOp          _initTransition   : 6;
        TransitionOp          _finalTransition  : 6;
        //u32                   _padding          : 12;
    };

    /**
    * RenderPassInfo struct
    * 36 byte size
    */
    struct RenderPassDescription
    {
        RenderPassDescription() noexcept
        {
            _attachments.fill(AttachmentDescription());
            _countColorAttachments = 0;
            _hasDepthStencilAttahment = false;
        }

        std::array<AttachmentDescription, k_maxFramebufferAttachments> _attachments; //32 bytes
        u32 _countColorAttachments      : 4;
        u32 _hasDepthStencilAttahment   : 1;

        u32 _padding                    : 27;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
