#pragma once

#include "Object.h"
#include "Render.h"
#include "RenderTargetState.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace stream
{
    class Stream;
} //namespace stream
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class RenderPipeline;
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

            bool operator==(const InputBinding &other) const
            {
                if (this != &other)
                {
                    return _binding == other._binding
                        && _rate == other._rate
                        && _stride == other._stride;
                }

                return true;
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
                , _offset(0)
            {
                static_assert(sizeof(InputAttribute) == 4, "wrong size");
            }

            InputAttribute(u32 binding, u32 stream, Format format, u32 offset) noexcept
                : _binding(binding)
                , _stream(stream)
                , _format(format)
                , _offset(offset)
            {
                static_assert(sizeof(InputAttribute) == 4, "wrong size");
            }

            bool operator==(const InputAttribute& other) const
            {
                if (this != &other)
                {
                    return _binding == other._binding
                        && _stream == other._stream
                        && _format == other._format
                        && _offset == other._offset;
                }

                return true;
            }

            u32          _binding   : 3; //k_maxVertexInputBindings
            u32          _stream    : 5;
            Format       _format    : 8;
            u32          _offset    : 16;
        };

        VertexInputAttributeDesc() noexcept;
        VertexInputAttributeDesc(const VertexInputAttributeDesc& desc) noexcept;
        VertexInputAttributeDesc(const std::initializer_list<VertexInputAttributeDesc::InputBinding>& inputBinding, const std::initializer_list<VertexInputAttributeDesc::InputAttribute>& inputAttributes) noexcept;

        bool operator==(const VertexInputAttributeDesc& other) const
        {
            if (this != &other)
            {
                return _countInputBindings == other._countInputBindings
                    && _countInputAttributes == other._countInputAttributes
                    && _inputBindings == other._inputBindings
                    && _inputAttributes == other._inputAttributes;
            }

            return true;
        }

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

        std::array<InputBinding, k_maxVertexInputBindings>      _inputBindings;
        std::array<InputAttribute, k_maxVertexInputAttributes>  _inputAttributes;
        u32                                                     _countInputBindings     : 16;
        u32                                                     _countInputAttributes   : 16;
    };

    inline VertexInputAttributeDesc::VertexInputAttributeDesc() noexcept
        : _countInputBindings(0)
        , _countInputAttributes(0)
    {
        static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
    }

    inline VertexInputAttributeDesc::VertexInputAttributeDesc(const VertexInputAttributeDesc& desc) noexcept
        : _inputBindings(desc._inputBindings)
        , _inputAttributes(desc._inputAttributes)
        , _countInputBindings(desc._countInputBindings)
        , _countInputAttributes(desc._countInputAttributes)
    {
        static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
    }

    inline VertexInputAttributeDesc::VertexInputAttributeDesc(const std::initializer_list<VertexInputAttributeDesc::InputBinding>& inputBinding, const std::initializer_list<VertexInputAttributeDesc::InputAttribute>& inputAttributes) noexcept
    {
        ASSERT(inputBinding.size() <= _inputBindings.size(), "out of range");
        _countInputBindings = static_cast<u32>(inputBinding.size());
        memcpy(_inputBindings.data(), inputBinding.begin(), sizeof(InputAttribute) * inputBinding.size());

        ASSERT(inputAttributes.size() <= _inputAttributes.size(), "out of range");
        _countInputAttributes = static_cast<u32>(inputAttributes.size());
        memcpy(_inputAttributes.data(), inputAttributes.begin(), sizeof(InputAttribute) * inputAttributes.size());

        static_assert(sizeof(VertexInputAttributeDesc) == sizeof(_inputBindings) + sizeof(_inputAttributes) + 4, "wrong size");
    }

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

            bool operator==(const RasterizationState& other) const
            {
                if (this != &other)
                {
                    return _polygonMode == other._polygonMode
                        && _frontFace == other._frontFace
                        && _cullMode == other._cullMode
                        && _discardRasterization == other._discardRasterization
                        && _depthBiasConstant == other._depthBiasConstant
                        && _depthBiasClamp == other._depthBiasClamp
                        && _depthBiasSlope == other._depthBiasSlope;
                }

                return true;
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
                , _depthBounds({ 0.0f })
            {
                static_assert(sizeof(DepthStencilState) == sizeof(math::float2) + 4, "wrong size");
            }

            bool operator==(const DepthStencilState& other) const
            {
                if (this != &other)
                {
                    return _compareOp == other._compareOp
                        && _depthTestEnable == other._depthTestEnable
                        && _depthWriteEnable == other._depthWriteEnable
                        && _stencilTestEnable == other._stencilTestEnable
                        && _depthBoundsTestEnable == other._depthBoundsTestEnable
                        && _depthBounds == other._depthBounds;
                }

                return true;
            }

            CompareOperation     _compareOp               : 4;
            u32                  _depthTestEnable         : 1;
            u32                  _depthWriteEnable        : 1;
            u32                  _stencilTestEnable       : 1;
            u32                  _depthBoundsTestEnable   : 1;
            u32                  _unused                  : 24;
            math::float2         _depthBounds;
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

                bool operator==(const ColorBlendAttachmentState& other) const
                {
                    if (this != &other)
                    {
                        return _blendOp == other._blendOp
                            && _srcBlendFacor == other._srcBlendFacor
                            && _dscBlendFacor == other._dscBlendFacor
                            && _alphaBlendOp == other._alphaBlendOp
                            && _srcAlphaBlendFacor == other._srcAlphaBlendFacor
                            && _dscAlphaBlendFacor == other._dscAlphaBlendFacor
                            && _colorWriteMask == other._colorWriteMask
                            && _colorBlendEnable == other._colorBlendEnable;
                    }

                    return true;
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
                : _constant({ 0.f })
                , _logicalOp(LogicalOperation::LogicalOp_And)
                , _logicalOpEnable(false)
                , _unused(0)
            {
                static_assert(sizeof(BlendState) == sizeof(_colorBlendAttachments) + sizeof(_constant) + 4, "wrong size");
            }

            bool operator==(const BlendState& other) const
            {
                if (this != &other)
                {
                    return _colorBlendAttachments == other._colorBlendAttachments
                        && _constant[0] == other._constant[0]
                        && _constant[1] == other._constant[1]
                        && _constant[2] == other._constant[2]
                        && _constant[3] == other._constant[3]
                        && _logicalOp == other._logicalOp
                        && _logicalOpEnable == other._logicalOpEnable;
                }

                return true;
            }

#define USE_MULTI_COLOR_BLEND_ATTACMENTS 1

#if USE_MULTI_COLOR_BLEND_ATTACMENTS
            std::array<ColorBlendAttachmentState, k_maxColorAttachments>    _colorBlendAttachments;
#else
            std::array<ColorBlendAttachmentState, 1>                        _colorBlendAttachments;
#endif
            f32                                                             _constant[4];
            LogicalOperation                                                _logicalOp          : 5;
            u32                                                             _logicalOpEnable    : 1;
            u32                                                             _unused             : 26;
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

            bool operator==(const VertexInputState& other) const
            {
                if (this != &other)
                {
                    return _inputAttributes == other._inputAttributes
                        && _primitiveTopology == other._primitiveTopology;
                }

                return true;
            }

            VertexInputAttributeDesc _inputAttributes;
            PrimitiveTopology        _primitiveTopology;
        };

        GraphicsPipelineStateDesc() noexcept = default;
        GraphicsPipelineStateDesc(const GraphicsPipelineStateDesc&) = default;
        GraphicsPipelineStateDesc& operator=(const GraphicsPipelineStateDesc&) = default;

        bool operator==(const GraphicsPipelineStateDesc& other) const
        {
            static_assert(sizeof(GraphicsPipelineStateDesc) == sizeof(VertexInputState) + sizeof(RasterizationState) + sizeof(BlendState) + sizeof(DepthStencilState), "wrong size");
            if (this != &other)
            {
#if 1 //fast check. All data must be packed tightly
                return memcmp(this, &other, sizeof(GraphicsPipelineStateDesc)) == 0;
#else
                return _vertexInputState == other._vertexInputState
                    && _depthStencilState == other._depthStencilState
                    && _rasterizationState == other._rasterizationState
                    && _blendState == other._blendState;
#endif
            }

            return true;
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

        /**
        * @brief setDepthCompareOp method. Depth-Stencil state
        * @param CompareOperation op [required]
        */
        void setDepthCompareOp(CompareOperation op);

        /**
        * @brief setDepthTest method. Depth-Stencil state
        * @param bool enable [required]
        */
        void setDepthTest(bool enable);

        /**
        * @brief setDepthWrite method. Depth-Stencil state
        * @param bool enable [required]
        */
        void setDepthWrite(bool enable);

        /**
        * @brief getDepthCompareOp method. Depth-Stencil state
        * @return CompareOperation
        */
        CompareOperation getDepthCompareOp() const;

        /**
        * @brief isDepthTestEnable method. Depth-Stencil state
        * @return bool
        */
        bool isDepthTestEnable() const;

        /**
        * @brief isDepthWriteEnable method. Depth-Stencil state
        * @return bool
        */
        bool isDepthWriteEnable() const;

        void setBlendEnable(u32 index, bool enable);
        void setColorBlendFactor(u32 index, BlendFactor src, BlendFactor dst);
        void setColorBlendOp(u32 index,BlendOperation op);
        void setAlphaBlendFactor(u32 index, BlendFactor src, BlendFactor dst);
        void setAlphaBlendOp(u32 index, BlendOperation op);

        bool isBlendEnable(u32 index) const;
        BlendFactor getSrcColorBlendFactor(u32 index) const;
        BlendFactor getDstColorBlendFactor(u32 index) const;
        BlendOperation getColorBlendOp(u32 index) const;
        BlendFactor getSrcAlphaBlendFactor(u32 index) const;
        BlendFactor getDstAlphaBlendFactor(u32 index) const;
        BlendOperation getAlphaBlendOp(u32 index) const;

        void setColorMask(u32 index, ColorMaskFlags colormask);
        void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

        ColorMaskFlags getColorMask(u32 index) const;
        PrimitiveTopology getPrimitiveTopology() const;

    public:

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const VertexInputAttributeDescription& attributeDesc [required]
        * @param onst RenderPassDesc& renderpassDesc [required]
        * @param const ShaderProgram* const program [required]
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(Device* device, const VertexInputAttributeDesc& attributeDesc, const RenderPassDesc& renderpassDesc, const ShaderProgram* const program, const std::string& name = "") noexcept;

        /**
        * @brief GraphicsPipelineState constructor.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const GraphicsPipelineStateDescription& pipelineDesc [required]
        * @param const RenderPassDesc& renderpassDesc [required]
        * @param const ShaderProgram* const program [required]
        * @param const std::string& name [optional]
        */
        explicit GraphicsPipelineState(Device* device, const GraphicsPipelineStateDesc& pipelineDesc, const RenderPassDesc& renderpassDesc, const ShaderProgram* const program, const std::string& name = "") noexcept;

        /**
        * @brief GraphicsPipelineState destructor
        */
        ~GraphicsPipelineState();

        const GraphicsPipelineStateDesc& getPipelineStateDesc() const;
        const RenderPassDesc& getRenderPassDesc() const;
        const ShaderProgram* getShaderProgram() const;
        const std::string& getName() const;

    private:

        GraphicsPipelineState() = delete;
        GraphicsPipelineState(const GraphicsPipelineState&) = delete;

        void destroyPipelines(const std::vector<RenderPipeline*>& pipelines);

        Device* const                   m_device;

    public:

        GraphicsPipelineStateDesc       m_pipelineStateDesc;
        const RenderPassDesc            m_renderPassDesc;
        const ShaderProgram*            m_program;
        ObjectTracker<RenderPipeline>   m_tracker;

        const std::string               m_name;
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

    inline void GraphicsPipelineState::setBlendEnable(u32 index, bool enable)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._colorBlendEnable = enable;
    }

    inline void GraphicsPipelineState::setColorBlendFactor(u32 index, BlendFactor src, BlendFactor dst)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._srcBlendFacor = src;
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._dscBlendFacor = dst;
    }

    inline void GraphicsPipelineState::setColorBlendOp(u32 index, BlendOperation op)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._blendOp = op;
    }

    inline void GraphicsPipelineState::setAlphaBlendFactor(u32 index, BlendFactor src, BlendFactor dst)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._srcAlphaBlendFacor = src;
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._dscAlphaBlendFacor = dst;
    }

    inline void GraphicsPipelineState::setAlphaBlendOp(u32 index, BlendOperation op)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._alphaBlendOp = op;
    }

    inline bool GraphicsPipelineState::isBlendEnable(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._colorBlendEnable;
    }

    inline BlendFactor GraphicsPipelineState::getSrcColorBlendFactor(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._srcBlendFacor;
    }

    inline BlendFactor GraphicsPipelineState::getDstColorBlendFactor(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._dscBlendFacor;
    }

    inline BlendOperation GraphicsPipelineState::getColorBlendOp(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._blendOp;
    }

    inline BlendFactor GraphicsPipelineState::getSrcAlphaBlendFactor(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._srcAlphaBlendFacor;
    }

    inline BlendFactor GraphicsPipelineState::getDstAlphaBlendFactor(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._dscAlphaBlendFacor;
    }

    inline BlendOperation GraphicsPipelineState::getAlphaBlendOp(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._alphaBlendOp;
    }

    inline void GraphicsPipelineState::setColorMask(u32 index, ColorMaskFlags colormask)
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[index]._colorWriteMask = colormask;
    }

    inline ColorMaskFlags GraphicsPipelineState::getColorMask(u32 index) const
    {
        return m_pipelineStateDesc._blendState._colorBlendAttachments[index]._colorWriteMask;
    }

    inline PrimitiveTopology GraphicsPipelineState::getPrimitiveTopology() const
    {
        return m_pipelineStateDesc._vertexInputState._primitiveTopology;
    }

    inline const GraphicsPipelineStateDesc& GraphicsPipelineState::getPipelineStateDesc() const
    {
        return m_pipelineStateDesc;
    }

    inline const RenderPassDesc& GraphicsPipelineState::getRenderPassDesc() const
    {
        return m_renderPassDesc;
    }

    inline const ShaderProgram* GraphicsPipelineState::getShaderProgram() const
    {
        return m_program;
    }

    inline const std::string& GraphicsPipelineState::getName() const
    {
        return m_name;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ComputePipelineState class. Game side.
    * Includes a shader program
    */
    class ComputePipelineState : public PipelineState
    {
    public:

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
        ~ComputePipelineState();

        const ShaderProgram* getShaderProgram() const;
        const std::string& getName() const;

    private:

        ComputePipelineState() = delete;
        ComputePipelineState(const ComputePipelineState&) = delete;

        void destroyPipelines(const std::vector<RenderPipeline*>& pipelines);

        Device&                         m_device;

    public:

        const ShaderProgram*            m_program;
        ObjectTracker<RenderPipeline>   m_tracker;

        const std::string               m_name;
    };

    inline const ShaderProgram* ComputePipelineState::getShaderProgram() const
    {
        return m_program;
    }

    inline const std::string& ComputePipelineState::getName() const
    {
        return m_name;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
