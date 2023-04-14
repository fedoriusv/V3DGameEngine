#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "BufferProperties.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief enum PolygonMode
    */
    enum PolygonMode : u16
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
    enum class FrontFace : u16
    {
        FrontFace_Clockwise,
        FrontFace_CounterClockwise
    };

    /**
    * @brief enum class CullMode
    */
    enum class CullMode : u16
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
    enum ColorMask : u8
    {
        ColorMask_None = 0,
        ColorMask_R = 0x1,
        ColorMask_G = 0x2,
        ColorMask_B = 0x4,
        ColorMask_A = 0x8,

        ColorMask_All = ColorMask_R | ColorMask_G | ColorMask_B | ColorMask_A

    };

    typedef u8 ColorMaskFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GraphicsPipelineStateDescription struct
    */
    struct GraphicsPipelineStateDescription
    {
        /**
        * @brief RasterizationState struct
        */
        struct RasterizationState
        {
            RasterizationState() noexcept
                : _polygonMode(PolygonMode::PolygonMode_Fill)
                , _frontFace(FrontFace::FrontFace_Clockwise)
                , _cullMode(CullMode::CullMode_None)
                , _discardRasterization(false)

                , _depthBiasConstant(0.f)
                , _depthBiasClamp(0.f)
                , _depthBiasSlope(0.f)
            {
                static_assert(sizeof(RasterizationState) == 20, "wrong size");
            }

            PolygonMode _polygonMode;
            FrontFace   _frontFace;
            CullMode    _cullMode;
            u16         _discardRasterization;

            f32         _depthBiasConstant;
            f32         _depthBiasClamp;
            f32         _depthBiasSlope;
        };

        /**
        * @brief ColorBlendAttachmentState struct
        */
        struct ColorBlendAttachmentState
        {
            ColorBlendAttachmentState() noexcept
                : _blendOp(BlendOperation::BlendOp_Add)
                , _srcBlendFacor(BlendFactor::BlendFactor_SrcColor)
                , _dscBlendFacor(BlendFactor::BlendFactor_DstColor)

                , _alphaBlendOp(BlendOperation::BlendOp_Add)
                , _srcAlphaBlendFacor(BlendFactor::BlendFactor_SrcAlpha)
                , _dscAlphaBlendFacor(BlendFactor::BlendFactor_DstAlpha)

                , _colorWriteMask(ColorMask::ColorMask_All)
                , _colorBlendEnable(false)

            {
                static_assert(sizeof(ColorBlendAttachmentState) == 32, "wrong size");
            }

            BlendOperation  _blendOp;
            BlendFactor     _srcBlendFacor;
            BlendFactor     _dscBlendFacor;

            BlendOperation  _alphaBlendOp;
            BlendFactor     _srcAlphaBlendFacor;
            BlendFactor     _dscAlphaBlendFacor;

            ColorMaskFlags  _colorWriteMask;
            bool            _colorBlendEnable;

            u8             _padding[3] = {};
        };

        /**
        * @brief DepthStencilState struct
        */
        struct DepthStencilState
        {
            DepthStencilState() noexcept
                : _compareOp(CompareOperation::CompareOp_Greater)
                , _depthTestEnable(false)
                , _depthWriteEnable(false)
                , _stencilTestEnable(false)
                , _depthBoundsTestEnable(false)
                , _depthBounds(math::Vector2D(0.0f))
            {
                static_assert(sizeof(DepthStencilState) == sizeof(math::Vector2D) + 8, "wrong size");
            }

            CompareOperation    _compareOp;
            bool                _depthTestEnable;
            bool                _depthWriteEnable;

            bool                _stencilTestEnable;

            bool                _depthBoundsTestEnable;
            math::Vector2D      _depthBounds;
        };

        /**
        * @brief BlendState struct
        */
        struct BlendState
        {
            BlendState() noexcept
                : _constant(math::Vector4D(0.0f))
                , _logicalOp(LogicalOperation::LogicalOp_And)
                , _logicalOpEnable(false)
            {
                static_assert(sizeof(BlendState) == sizeof(ColorBlendAttachmentState) + sizeof(math::Vector4D) + 8, "wrong size");
            }

            ColorBlendAttachmentState _colorBlendAttachments;
            //std::array<ColorBlendAttachmentState, k_maxFramebufferAttachments> _colorBlendAttachments;

            math::Vector4D      _constant;
            LogicalOperation    _logicalOp;
            bool                _logicalOpEnable;

            u8                  _padding[3] = {};
        };

        /**
        * @brief VertexInputState struct
        */
        struct VertexInputState
        {
            VertexInputState() noexcept
                : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
            {
                static_assert(sizeof(VertexInputState) == sizeof(VertexInputAttributeDescription) + 4, "wrong size");
            }

            VertexInputAttributeDescription _inputAttributes;
            PrimitiveTopology               _primitiveTopology;
        };

        GraphicsPipelineStateDescription() noexcept = default;
        GraphicsPipelineStateDescription(const GraphicsPipelineStateDescription&) = default;
        GraphicsPipelineStateDescription& operator=(const GraphicsPipelineStateDescription&) = default;

        bool operator==(const GraphicsPipelineStateDescription& op) const
        {
            if (this == &op)
            {
                return true;
            }

            static_assert(sizeof(GraphicsPipelineStateDescription) == sizeof(VertexInputState) + sizeof(RasterizationState) + sizeof(BlendState) + sizeof(DepthStencilState), "wrong size");
            return memcmp(this, &op, sizeof(GraphicsPipelineStateDescription)) == 0;
        }

        VertexInputState    _vertexInputState;
        RasterizationState  _rasterizationState;
        BlendState          _blendState;
        DepthStencilState   _depthStencilState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
