#pragma once

#include "Common.h"
#include "Object.h"
#include "Formats.h"
#include "Color.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////Limits////////////////////////////////////////////////////

    /**
    * @brief k_maxColorAttachments
    */
    constexpr u32 k_maxColorAttachments = 8;

    /**
    * @brief k_maxVertexInputAttributes
    */
    constexpr u32 k_maxVertexInputAttributes = 8;

    /**
    * @brief k_maxVertexInputBindings
    */
    constexpr u32 k_maxVertexInputBindings = 4;

    /**
    * @brief k_maxDescriptorSetCount
    */
    constexpr u32 k_maxDescriptorSetCount = 4;

    /**
    * @brief k_maxDescriptorBindingCount
    */
    constexpr u32 k_maxDescriptorBindingCount = 8;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureTarget enum. Contains a texture target
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
    * @brief RenderTargetLoadOp enum. Loads operations for a render target
    */
    enum class RenderTargetLoadOp : u32
    {
        LoadOp_DontCare,
        LoadOp_Clear,
        LoadOp_Load,
    };

    /**
    * @brief RenderTargetStoreOp enum. Stores operations for a render target
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
        TransitionOp_ColorAttachment,
        TransitionOp_DepthStencilAttachment,

        TransitionOp_GeneralGraphic,
        TransitionOp_GeneralCompute,

        TransitionOp_Present,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief k_generalLayer index for all layers
    */
    constexpr u32 k_generalLayer = ~0;

    /**
    * @brief k_allMipmapsLevels index for all mipmaps
    */
    constexpr u32 k_allMipmapsLevels = ~0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IndexBufferType enum class.
    */
    enum class IndexBufferType : u32
    {
        IndexType_16,
        IndexType_32
    };

    enum class InputRate : u32
    {
        InputRate_Vertex = 0,
        InputRate_Instance = 1
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief enum PolygonMode
    */
    enum PolygonMode : u32
    {
        PolygonMode_Fill,
        PolygonMode_Line,
        PolygonMode_Point
    };

    /**
    * @brief enum PrimitiveTopology
    */
    enum PrimitiveTopology : u32
    {
        PrimitiveTopology_PointList,
        PrimitiveTopology_LineList,
        PrimitiveTopology_LineStrip,
        PrimitiveTopology_TriangleList,
        PrimitiveTopology_TriangleStrip,
        PrimitiveTopology_TriangleFan,
        PrimitiveTopology_LineListWithAdjacency,
        PrimitiveTopology_LineStripWithAdjacency,
        PrimitiveTopology_TriangleListWithAdjacency,
        PrimitiveTopology_TriangleStripWithAdjacency,
        PrimitiveTopology_PatchList
    };

    /**
    * @brief enum class FrontFace
    */
    enum class FrontFace : u32
    {
        FrontFace_Clockwise,
        FrontFace_CounterClockwise
    };

    /**
    * @brief enum class CullMode
    */
    enum class CullMode : u32
    {
        CullMode_None,
        CullMode_Front,
        CullMode_Back,
        CullMode_FrontAndBack,
    };

    /**
    * @brief enum class BlendOperation
    */
    enum class BlendOperation : u32
    {
        BlendOp_Add = 0,
        BlendOp_Subtract = 1,
        BlendOp_ReverseSubtract = 2,
        BlendOp_Min = 3,
        BlendOp_Max = 4,
    };

    /**
    * @brief enum class BlendFactor
    */
    enum class BlendFactor : u32
    {
        BlendFactor_Zero = 0,
        BlendFactor_One = 1,
        BlendFactor_SrcColor = 2,
        BlendFactor_OneMinusSrcColor = 3,
        BlendFactor_DstColor = 4,
        BlendFactor_OneMinusDstColor = 5,
        BlendFactor_SrcAlpha = 6,
        BlendFactor_OneMinusSrcAlpha = 7,
        BlendFactor_DstAlpha = 8,
        BlendFactor_OneMinusDstAlpha = 9,
        BlendFactor_ConstantColor = 10,
        BlendFactor_OneMinusConstantColor = 11,
        BlendFactor_ConstantAlpha = 12,
        BlendFactor_OneMinusConstantAlpha = 13,
        BlendFactor_SrcAlphaSaturate = 14,
        BlendFactor_Src1Color = 15,
        BlendFactor_OneMinusSrc1Color = 16,
        BlendFactor_Src1Alpha = 17,
        BlendFactor_OneMinusSrc1Alpha = 18,
    };

    /**
    * @brief enum class LogicalOperation
    */
    enum class LogicalOperation : u32
    {
        LogicalOp_Clear = 0,
        LogicalOp_And = 1,
        LogicalOp_AndReverse = 2,
        LogicalOp_Copy = 3,
        LogicalOp_AndInverted = 4,
        LogicalOp_NoOp = 5,
        LogicalOp_Xor = 6,
        LogicalOp_Or = 7,
        LogicalOp_Nor = 8,
        LogicalOp_Equivalent = 9,
        LogicalOp_Invert = 10,
        LogicalOp_OrReverse = 11,
        LogicalOp_CopyInverted = 12,
        LogicalOp_OrInverted = 13,
        LogicalOp_Nand = 14,
        LogicalOp_Set = 15,
    };

    /**
    * @brief enum class CompareOperation
    */
    enum class CompareOperation : u32
    {
        CompareOp_Never = 0,
        CompareOp_Less = 1,
        CompareOp_Equal = 2,
        CompareOp_LessOrEqual = 3,
        CompareOp_Greater = 4,
        CompareOp_NotEqual = 5,
        CompareOp_GreaterOrEqual = 6,
        CompareOp_Always = 7,
    };

    /**
    * @brief enum class ColorMask
    */
    enum ColorMask : u32
    {
        ColorMask_None = 0x0,
        ColorMask_R = 0x1,
        ColorMask_G = 0x2,
        ColorMask_B = 0x4,
        ColorMask_A = 0x8,

        ColorMask_All = ColorMask_R | ColorMask_G | ColorMask_B | ColorMask_A

    };

    /**
    * @brief ColorMaskFlags
    */
    typedef u32 ColorMaskFlags;

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
    * @brief TextureUsage enum. Flags describe a texture purposes
    */
    enum TextureUsage : u32
    {
        TextureUsage_Write              = 0x1,      //Write to texture
        TextureUsage_Read               = 0x2,      //Read from texture

        TextureUsage_Sampled            = 0x4,      //Texture is shader visible, read only
        TextureUsage_Attachment         = 0x8,      //Texture is render target
        TextureUsage_Storage            = 0x10,     //Shader storage

        TextureUsage_Resolve            = 0x20,     //Multisample render target
        TextureUsage_GenerateMipmaps    = 0x40,     //Internaly generate mipmaps for rendertarget
        TextureUsage_Backbuffer         = 0x80,     //Swapchain flag, read only flag

        TextureUsage_Shared             = 0x100,    //Share data between GPU/CPU side
    };

    /**
    * @brief TextureUsageFlags. Combination of TextureUsage enum
    */
    typedef u32 TextureUsageFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderTexture base class. Render side
    */
    class RenderTexture : public utils::ResourceID<RenderTexture, u64>
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

        static_assert(sizeof(Subresource) == 8, "Wrong size");


        [[nodiscard]] static const Subresource makeSubresource(u32 baseLayer, u32 layers, u32 baseMip, u32 mips)
        {
            ASSERT(baseLayer != ~0 && baseMip != ~0 && layers != ~0 && mips != ~0, "must be real");
            return { baseLayer, layers, baseMip, mips };
        }

        virtual bool hasUsageFlag(TextureUsage usage) const = 0;

    protected:

        RenderTexture() = default;
        virtual ~RenderTexture() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    using TextureHandle = ObjectHandle<void*>;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BufferUsage enum. usageFlag inside Buffer
    */
    enum BufferUsage : u32
    {
        Buffer_Write    = 0x01,     //Write to buffer
        Buffer_Read     = 0x02,     //Read from buffer

        Buffer_Dynamic  = 0x04,     //Stream read/write buffer
    };

    /**
    * @brief BufferUsageFlags. Combination of BufferUsage enum
    */
    typedef u32 BufferUsageFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderBuffer base class. Render side
    */
    class RenderBuffer : public utils::ResourceID<RenderBuffer, u64>
    {
    public:

        enum class Type : u32
        {
            VertexBuffer,
            IndexBuffer,
            ConstantBuffer,
            StagingBuffer,
            Readback
        };

        virtual bool hasUsageFlag(BufferUsage usage) const = 0;

    protected:

        RenderBuffer() = default;
        virtual ~RenderBuffer() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    using HandleBuffer = ObjectHandle<RenderBuffer*>;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
