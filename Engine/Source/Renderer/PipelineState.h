#pragma once

#include "Object.h"
#include "Render.h"

//#include "ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class Pipeline;
    class RenderTargetState;
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexInputAttributeDesc struct
    */
    struct VertexInputAttributeDesc
    {
        /**
        * @brief InputBinding struct
        */
        struct InputBinding
        {
            InputBinding() noexcept
                : _binding(0)
                , _rate(InputRate::InputRate_Vertex)
                , _stride(0)
            {
                static_assert(sizeof(InputBinding) == 4, "wrong size");
            }
            
            InputBinding(u32 binding, InputRate rate, u32 stride) noexcept
                : _binding(binding)
                , _rate(rate)
                , _stride(stride)
            {
                static_assert(sizeof(InputBinding) == 4, "wrong size");
            }

            u32       _binding : 3; //k_maxVertexInputBindings
            InputRate _rate    : 1;
            u32       _stride  : 12;
        };

        /**
        * @brief InputAttribute struct
        */
        struct InputAttribute
        {
            InputAttribute() noexcept
                : _binding(0)
                , _stream(0)
                , _format(Format::Format_Undefined)
                , _offest(0)
            {
                static_assert(sizeof(InputAttribute) == 4, "wrong size");
            }

            InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept
                : _binding(binding)
                , _stream(stream)
                , _format(format)
                , _offest(offset)
            {
                static_assert(sizeof(InputAttribute) == 4, "wrong size");
            }

            u32          _binding   : 3; //k_maxVertexInputBindings
            u32          _stream    : 5;
            Format       _format    : 8;
            u32          _offest    : 16;
        };

        VertexInputAttributeDesc() noexcept
            : _countInputAttributes(0)
        {
            static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
        }

        VertexInputAttributeDesc(const VertexInputAttributeDesc& desc) noexcept
            : _countInputAttributes(desc._countInputAttributes)
            , _inputAttributes(desc._inputAttributes)
        {
            static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
        }

        VertexInputAttributeDesc(const std::initializer_list<VertexInputAttributeDesc::InputBinding>& inputBinding, const std::initializer_list<VertexInputAttributeDesc::InputAttribute>& inputAttributes) noexcept
        {
            ASSERT(_inputBindings.size() <= inputBinding.size(), "out of range");
            _countInputBindings = static_cast<u32>(inputBinding.size());
            memcpy(_inputBindings.data(), inputBinding.begin(), sizeof(InputAttribute) * inputBinding.size());

            ASSERT(inputAttributes.size() <= inputAttributes.size(), "out of range");
            _countInputAttributes = static_cast<u32>(inputAttributes.size());
            memcpy(_inputAttributes.data(), inputAttributes.begin(), sizeof(InputAttribute) * inputAttributes.size());

            static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
        }

        std::array<InputBinding, k_maxVertexInputBindings>      _inputBindings;
        std::array<InputAttribute, k_maxVertexInputAttributes>  _inputAttributes;
        u32                                                     _countInputBindings     : 16;
        u32                                                     _countInputAttributes   : 16;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GraphicsPipelineStateDesc struct
    */
    struct GraphicsPipelineStateDesc
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
                , _unused(0)

                , _depthBiasConstant(0.f)
                , _depthBiasClamp(0.f)
                , _depthBiasSlope(0.f)
            {
                static_assert(sizeof(RasterizationState) == 16, "wrong size");
            }

            PolygonMode _polygonMode            : 2;
            FrontFace   _frontFace              : 1;
            CullMode    _cullMode               : 3;
            u32         _discardRasterization   : 1;
            u32         _unused                 : 25;

            f32         _depthBiasConstant;
            f32         _depthBiasClamp;
            f32         _depthBiasSlope;
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
                , _unused(0)
                , _depthBounds(math::Vector2D(0.0f))
            {
                static_assert(sizeof(DepthStencilState) == sizeof(math::Vector2D) + 8, "wrong size");
            }

            CompareOperation   _compareOp               : 4;
            u32                _depthTestEnable         : 1;
            u32                _depthWriteEnable        : 1;
            u32                _stencilTestEnable       : 1;
            u32                _depthBoundsTestEnable   : 1;
            u32                _unused                  : 27;
            math::Vector2D     _depthBounds;
        };

        /**
        * @brief BlendState struct
        */
        struct BlendState
        {
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
                    , _unused(0)
                {
                    static_assert(sizeof(ColorBlendAttachmentState) == 8, "wrong size");
                }

                BlendOperation  _blendOp            : 4;
                BlendFactor     _srcBlendFacor      : 6;
                BlendFactor     _dscBlendFacor      : 6;
                BlendOperation  _alphaBlendOp       : 4;
                BlendFactor     _srcAlphaBlendFacor : 6;
                BlendFactor     _dscAlphaBlendFacor : 6;
                ColorMaskFlags  _colorWriteMask     : 4;
                u32             _colorBlendEnable   : 1;
                u32             _unused             : 27;
            };

            BlendState() noexcept
                : _constant(math::Vector4D(0.0f))
                , _logicalOp(LogicalOperation::LogicalOp_And)
                , _logicalOpEnable(false)
                , _unused(0)
            {
                static_assert(sizeof(BlendState) == sizeof(_colorBlendAttachments) + sizeof(math::Vector4D) + 8, "wrong size");
            }

#if USE_MULTI_COLOR_BLEND_ATTACMENTS
            std::array<ColorBlendAttachmentState, k_maxColorAttachments>    _colorBlendAttachments;
#else
            std::array<ColorBlendAttachmentState, 1>                        _colorBlendAttachments;
#endif
            math::Vector4D                                                  _constant;
            LogicalOperation                                                _logicalOp          : 5;
            u32                                                             _logicalOpEnable    : 1;
            u32                                                             _unused             : 28;
        };

        /**
        * @brief VertexInputState struct
        */
        struct VertexInputState
        {
            VertexInputState() noexcept
                : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
            {
                static_assert(sizeof(VertexInputState) == sizeof(VertexInputAttributeDesc) + sizeof(PrimitiveTopology), "wrong size");
            }

            VertexInputAttributeDesc _inputAttributes;
            PrimitiveTopology        _primitiveTopology;
        };

        GraphicsPipelineStateDesc() noexcept = default;
        GraphicsPipelineStateDesc(const GraphicsPipelineStateDesc&) = default;
        GraphicsPipelineStateDesc& operator=(const GraphicsPipelineStateDesc&) = default;

        bool operator==(const GraphicsPipelineStateDesc& op) const
        {
            if (this == &op)
            {
                return true;
            }

            static_assert(sizeof(GraphicsPipelineStateDesc) == sizeof(VertexInputState) + sizeof(RasterizationState) + sizeof(BlendState) + sizeof(DepthStencilState), "wrong size");
            return memcmp(this, &op, sizeof(GraphicsPipelineStateDesc)) == 0;
        }

        VertexInputState    _vertexInputState;
        DepthStencilState   _depthStencilState;
        RasterizationState  _rasterizationState;
        BlendState          _blendState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PipelineState base class. Game side
    */
    class PipelineState : public Object
    {
    protected:

        PipelineState() noexcept = default;
        virtual ~PipelineState() = default;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GraphicsPipelineState class. Game side.
    * Includes a shader program and render states
    */
    class GraphicsPipelineState : public PipelineState
    {
    public:

        /**
        * @brief setPolygonMode method. Rasterization state
        * @param PolygonMode polygonMode [required]
        */
        void setPolygonMode(PolygonMode polygonMode);

        /**
        * @brief setFrontFace method. Rasterization state
        * @param FrontFace frontFace [required]
        */
        void setFrontFace(FrontFace frontFace);

        /**
        * @brief setCullMode method. Rasterization state
        * @param CullMode cullMode [required]
        */
        void setCullMode(CullMode cullMode);

        /**
        * @brief setDepthBias method. Rasterization state
        * @param f32 constantFactor [required]
        * @param f32 clamp [required]
        * @param f32 slopeFactor [required]
        */
        void setDepthBias(f32 constantFactor, f32 clamp, f32 slopeFactor);

        /**
        * @brief setDisacardRasterization method. Rasterization state
        * @param bool disabled [required]
        */
        void setDisacardRasterization(bool disabled);

        /**
        * @brief getPolygonMode method. Rasterization state
        * @return PolygonMode
        */
        PolygonMode getPolygonMode() const;

        /**
        * @brief getFrontFace method. Rasterization state
        * @return FrontFace
        */
        FrontFace getFrontFace() const;

        /**
        * @brief getCullMode method. Rasterization state
        * @return CullMode
        */
        CullMode getCullMode() const;

        void setDepthCompareOp(CompareOperation op);
        void setDepthTest(bool enable);
        void setDepthWrite(bool enable);

        CompareOperation getDepthCompareOp() const;
        bool isDepthTestEnable() const;
        bool isDepthWriteEnable() const;

        void setBlendEnable(bool enable);
        void setColorBlendFactor(BlendFactor src, BlendFactor dst);
        void setColorBlendOp(BlendOperation op);
        void setAlphaBlendFactor(BlendFactor src, BlendFactor dst);
        void setAlphaBlendOp(BlendOperation op);

        bool isBlendEnable() const;
        BlendFactor getSrcColorBlendFactor() const;
        BlendFactor getDstColorBlendFactor() const;
        BlendOperation getColorBlendOp() const;
        BlendFactor getSrcAlphaBlendFactor() const;
        BlendFactor getDstAlphaBlendFactor() const;
        BlendOperation getAlphaBlendOp() const;

        void setColorMask(ColorMaskFlags colormask);
        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        ColorMaskFlags getColorMask() const;
        PrimitiveTopology getPrimitiveTopology() const;

    public:

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const VertexInputAttributeDescription& vertex [required]
        * @param const ShaderProgram* const program [required]
        * @param onst RenderTargetState* const renderTaget [required]
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(Device* devive, const VertexInputAttributeDesc& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name = "") noexcept;

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const GraphicsPipelineStateDescription& desc [required]
        * @param const ShaderProgram* const program [required]
        * @param const RenderTargetState* const renderTaget [required]
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(Device* devive, const GraphicsPipelineStateDesc& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name = "") noexcept;

        /**
        * @brief GraphicsPipelineState destructor
        */
        ~GraphicsPipelineState() = default;

    private:

        GraphicsPipelineState() = delete;
        GraphicsPipelineState(const GraphicsPipelineState&) = delete;

        Device* const               m_device;
        GraphicsPipelineStateDesc   m_pipelineStateDesc;
        const ShaderProgram*        m_program;
        const RenderTargetState*    m_renderTaget;
    };

    inline void GraphicsPipelineState::setPolygonMode(PolygonMode polygonMode)
    {
        m_pipelineStateDesc._rasterizationState._polygonMode = polygonMode;
    }

    inline void GraphicsPipelineState::setFrontFace(FrontFace frontFace)
    {
        m_pipelineStateDesc._rasterizationState._frontFace = frontFace;
    }

    inline void GraphicsPipelineState::setCullMode(CullMode cullMode)
    {
        m_pipelineStateDesc._rasterizationState._cullMode = cullMode;
    }

    inline void GraphicsPipelineState::setDepthBias(f32 constantFactor, f32 clamp, f32 slopeFactor)
    {
        m_pipelineStateDesc._rasterizationState._depthBiasConstant = constantFactor;
        m_pipelineStateDesc._rasterizationState._depthBiasClamp = clamp;
        m_pipelineStateDesc._rasterizationState._depthBiasSlope = slopeFactor;
    }

    inline void GraphicsPipelineState::setDisacardRasterization(bool disabled)
    {
        m_pipelineStateDesc._rasterizationState._discardRasterization = disabled;
    }

    inline PolygonMode GraphicsPipelineState::getPolygonMode() const
    {
        return m_pipelineStateDesc._rasterizationState._polygonMode;
    }

    inline FrontFace GraphicsPipelineState::getFrontFace() const
    {
        return m_pipelineStateDesc._rasterizationState._frontFace;
    }

    inline CullMode GraphicsPipelineState::getCullMode() const
    {
        return m_pipelineStateDesc._rasterizationState._cullMode;
    }

    inline void GraphicsPipelineState::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
    {
        m_pipelineStateDesc._vertexInputState._primitiveTopology = primitiveTopology;
    }

    inline void GraphicsPipelineState::setDepthCompareOp(CompareOperation op)
    {
        m_pipelineStateDesc._depthStencilState._compareOp = op;
    }

    inline void GraphicsPipelineState::setDepthTest(bool enable)
    {
        m_pipelineStateDesc._depthStencilState._depthTestEnable = enable;
    }

    inline void GraphicsPipelineState::setDepthWrite(bool enable)
    {
        m_pipelineStateDesc._depthStencilState._depthWriteEnable = enable;
    }

    inline CompareOperation GraphicsPipelineState::getDepthCompareOp() const
    {
        return m_pipelineStateDesc._depthStencilState._compareOp;
    }

    inline bool GraphicsPipelineState::isDepthTestEnable() const
    {
        return m_pipelineStateDesc._depthStencilState._depthTestEnable;
    }

    inline bool GraphicsPipelineState::isDepthWriteEnable() const
    {
        return m_pipelineStateDesc._depthStencilState._depthWriteEnable;
    }

    inline void GraphicsPipelineState::setBlendEnable(bool enable)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._colorBlendEnable = enable;
    }

    inline void GraphicsPipelineState::setColorBlendFactor(BlendFactor src, BlendFactor dst)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._srcBlendFacor = src;
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._dscBlendFacor = dst;
    }

    inline void GraphicsPipelineState::setColorBlendOp(BlendOperation op)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._blendOp = op;
    }

    inline void GraphicsPipelineState::setAlphaBlendFactor(BlendFactor src, BlendFactor dst)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._srcAlphaBlendFacor = src;
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._dscAlphaBlendFacor = dst;
    }

    inline void GraphicsPipelineState::setAlphaBlendOp(BlendOperation op)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._alphaBlendOp = op;
    }

    inline bool GraphicsPipelineState::isBlendEnable() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._colorBlendEnable;
    }

    inline BlendFactor GraphicsPipelineState::getSrcColorBlendFactor() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._srcBlendFacor;
    }

    inline BlendFactor GraphicsPipelineState::getDstColorBlendFactor() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._dscBlendFacor;
    }

    inline BlendOperation GraphicsPipelineState::getColorBlendOp() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._blendOp;
    }

    inline BlendFactor GraphicsPipelineState::getSrcAlphaBlendFactor() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._srcAlphaBlendFacor;
    }

    inline BlendFactor GraphicsPipelineState::getDstAlphaBlendFactor() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._dscAlphaBlendFacor;
    }

    inline BlendOperation GraphicsPipelineState::getAlphaBlendOp() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._alphaBlendOp;
    }

    inline void GraphicsPipelineState::setColorMask(ColorMaskFlags colormask)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[0]._colorWriteMask = colormask;
    }

    inline ColorMaskFlags GraphicsPipelineState::getColorMask() const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[0]._colorWriteMask;
    }

    inline PrimitiveTopology GraphicsPipelineState::getPrimitiveTopology() const
    {
        return m_pipelineStateDesc._vertexInputState._primitiveTopology;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ComputePipelineState class. Game side.
    * Includes a shader program
    */
    class ComputePipelineState : public PipelineState
    {
    private:

        /**
        * @brief ComputePipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const ShaderProgram* const program [required]
        * @param const std::string& name [optional]
        */
        explicit ComputePipelineState(Device* device, const ShaderProgram* program, const std::string& name = "") noexcept;

        /**
        * @brief ComputePipelineState destructor
        */
        ~ComputePipelineState() = default;

        ComputePipelineState() = delete;
        ComputePipelineState(const ComputePipelineState&) = delete;

        Device* const           m_device;
        const ShaderProgram*    m_program;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
