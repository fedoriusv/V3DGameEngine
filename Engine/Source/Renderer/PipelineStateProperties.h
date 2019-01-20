#pragma once

#include "Common.h"
#include "TextureProperties.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class PolygonMode : u32
    {
        PolygonMode_Triangle,
        PolygonMode_Line,
        PolygonMode_Point
    };

    enum class PrimitiveTopology : u32
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

    enum class FrontFace : u32
    {
        FrontFace_CounterClockwise,
        FrontFace_Clockwise
    };

    enum class CullMode : u32
    {
        CullMode_None,
        CullMode_Front,
        CullMode_Back,
        CullMode_FrontAndBack,
    };

    enum class BlendOperation : u32
    {
        BlendOp_Add = 0,
        BlendOp_Subtract = 1,
        BlendOp_ReverseSubtract = 2,
        BlendOp_Min = 3,
        BlendOp_Max = 4,
    };

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

    enum ColorMask : u16
    {
        ColorMask_None = 0,
        ColorMask_R = 0x1,
        ColorMask_G = 0x2,
        ColorMask_B = 0x4,
        ColorMask_A = 0x8,

        ColorMask_All = ColorMask_R | ColorMask_G | ColorMask_B | ColorMask_A

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * GraphicsPipelineStateDescription
    */
    struct GraphicsPipelineStateDescription
    {
        struct RasterizationState
        {
            RasterizationState() 
                : _polygonMode(PolygonMode::PolygonMode_Triangle)
                , _frontFace(FrontFace::FrontFace_Clockwise)
                , _cullMode(CullMode::CullMode_Back)
            {
            }

            PolygonMode _polygonMode;
            FrontFace   _frontFace;
            CullMode    _cullMode;
        };

        struct ColorBlendAttachmentState
        {
            ColorBlendAttachmentState()
                : _blendOp(BlendOperation::BlendOp_Add)
                , _srcBlendFacor(BlendFactor::BlendFactor_SrcColor)
                , _dscBlendFacor(BlendFactor::BlendFactor_DstColor)

                , _alphaBlendOp(BlendOperation::BlendOp_Add)
                , _srcAlphaBlendFacor(BlendFactor::BlendFactor_SrcAlpha)
                , _dscAlphaBlendFacor(BlendFactor::BlendFactor_DstAlpha)

                , _colorWriteMask(ColorMask::ColorMask_None)
                , _colorBlendEnable(false)
            {
            }

            BlendOperation  _blendOp;
            BlendFactor     _srcBlendFacor;
            BlendFactor     _dscBlendFacor;

            BlendOperation  _alphaBlendOp;
            BlendFactor     _srcAlphaBlendFacor;
            BlendFactor     _dscAlphaBlendFacor;

            u16             _colorWriteMask;
            bool            _colorBlendEnable;

            u8              _padding;
        };

        struct DepthStencilState
        {
            DepthStencilState()
                : _compareOp(CompareOperation::CompareOp_Greater)
                , _depthTestEnable(false)
                , _depthWriteEnable(false)
                , _stencilTestEnable(false)
                , _depthBoundsTestEnable(false)
                , _depthBounds(core::Vector2D(0.0f))
            {
            }

            CompareOperation    _compareOp;
            bool                _depthTestEnable;
            bool                _depthWriteEnable;

            bool                _stencilTestEnable;

            bool                _depthBoundsTestEnable;
            core::Vector2D      _depthBounds;
        };

        struct BlendState
        {
            BlendState()
                : _constant(core::Vector4D(0.0f))
                , _logicalOp(LogicalOperation::LogicalOp_And)
                , _logicalOpEnable(false)
            {
            }

            ColorBlendAttachmentState _colorBlendAttachments;
            //std::array<ColorBlendAttachmentState, k_maxFramebufferAttachments> _colorBlendAttachments;

            core::Vector4D      _constant;
            LogicalOperation    _logicalOp;
            bool                _logicalOpEnable;

            u8                  _padding[3];
        };

        GraphicsPipelineStateDescription()
            : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
        {
        }

        RasterizationState  _rasterizationState;
        BlendState          _blendState;
        DepthStencilState   _depthStencilState;

        PrimitiveTopology   _primitiveTopology;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
