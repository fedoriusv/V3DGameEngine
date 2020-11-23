#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureUsage enum. Flags describe a texture purposes
    */
    enum TextureUsage : u16
    {
        TextureUsage_Write = 0x01,      //Write to texture
        TextureUsage_Read = 0x02,       //Read from texture

        TextureUsage_Sampled = 0x04,    //Shader visible
        TextureUsage_Attachment = 0x08, //Render target

        TextureUsage_Resolve = 0x10,    //Multisample render target

        TextureUsage_Shared = 0x40,     //Share data between Game/Render side
    };

    /**
    * @brief TextureUsageFlags. Combination of TextureUsage enum
    */
    typedef u16 TextureUsageFlags;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureTarget enum. Contain a texture target
    */
    enum class TextureTarget : u16
    {
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCubeMap,
    };

    /**
    * @brief TextureSamples enum. Count of samples
    */
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

    /**
    * @brief RenderTargetLoadOp enum. Load operations for a render target
    */
    enum class RenderTargetLoadOp : u32
    {
        LoadOp_DontCare,
        LoadOp_Clear,
        LoadOp_Load,
    };

    /**
    * @brief RenderTargetStoreOp enum. Store operations for a render target
    */
    enum class RenderTargetStoreOp : u32
    {
        StoreOp_DontCare,
        StoreOp_Store,
    };

    /**
    * @brief TransitionOp enum. A transition operation detects how texture will be used.
    * Helps switch memory barriers for attachments internally
    */
    enum class TransitionOp : u32
    {
        TransitionOp_Undefined,
        TransitionOp_ShaderRead,
        TransitionOp_ColorAttachmet,
        TransitionOp_DepthStencilAttachmet,

        TransitionOp_Present,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief maximum color attachments inside a render target
    */
    constexpr u32 k_maxColorAttachments = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AttachmentDescription struct. Uses inside a render target.
    * 4 bytes size
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
        TransitionOp          _initTransition   : 3;
        TransitionOp          _finalTransition  : 3;
        u32                   _backbuffer       : 1;
        u32                   _autoResolve      : 1;
        u32                   _layer            : 3;

        u32                   _padding          : 2;

        static s32 uncompressLayer(u32 layer)
        {
            return (layer == 0x07) ? -1 : static_cast<s32>(layer);
        }
    };

    /**
    * @brief RenderPassDescription struct. Uses inside a render pass.
    * 40 bytes size
    */
    struct RenderPassDescription
    {
        RenderPassDescription() noexcept;
        bool operator==(const RenderPassDescription& other) const;

        struct RenderPassDesc
        {
            std::array<AttachmentDescription, k_maxColorAttachments + 1> _attachments; //36 bytes
            u32 _countColorAttachments      : 4;
            u32 _hasDepthStencilAttahment   : 1;

            u32 _padding                    : 27;
        };

        RenderPassDesc _desc;

        struct Hash
        {
            u32 operator()(const RenderPassDescription& desc) const;
        };

        struct Compare
        {
            bool operator()(const RenderPassDescription& op1, const RenderPassDescription& op2) const;
        };
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
