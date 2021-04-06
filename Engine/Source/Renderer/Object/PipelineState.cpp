#include "PipelineState.h"
#include "ShaderProgram.h"
#include "Renderer/Context.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

PipelineState::PipelineState(CommandList& cmdList) noexcept
    : m_cmdList(cmdList)
    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
}

PipelineState::~PipelineState()
{
    LOG_DEBUG("PipelineState::PipelineState destructor %llx", this);
    m_tracker.release();
}

void PipelineState::destroyPipelines(const std::vector<Pipeline*>& pipelines)
{
    if (m_cmdList.isImmediate())
    {
        for (auto& pipeline : pipelines)
        {
            m_cmdList.getContext()->removePipeline(pipeline);
        }
    }
    else
    {
        /*CommandRemovePipelines*/
        class CommandRemovePipelines final : public Command
        {
        public:

            explicit CommandRemovePipelines(const std::vector<Pipeline*>& pipelines) noexcept
                : m_pipelines(pipelines)
            {
                LOG_DEBUG("CommandRemovePipelines constructor");
            };

            CommandRemovePipelines() = delete;
            CommandRemovePipelines(CommandRemovePipelines&) = delete;

            ~CommandRemovePipelines()
            {
                LOG_DEBUG("CommandRemovePipelines destructor");
            };

            void execute(const CommandList& cmdList)
            {
                LOG_DEBUG("CommandRemovePipelines execute");
                for (auto& pipeline : m_pipelines)
                {
                    cmdList.getContext()->removePipeline(pipeline);
                }
            }

        private:

            const std::vector<Pipeline*> m_pipelines;
        };

        m_cmdList.pushCommand(new CommandRemovePipelines(pipelines));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept
    : PipelineState(cmdList)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
    m_pipelineStateDesc._vertexInputState._inputAttributes = vertex;
}

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget) noexcept
    : PipelineState(cmdList)
    , m_pipelineStateDesc(desc)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
}

void GraphicsPipelineState::setPolygonMode(PolygonMode polygonMode)
{
    m_pipelineStateDesc._rasterizationState._polygonMode = polygonMode;
}

void GraphicsPipelineState::setFrontFace(FrontFace frontFace)
{
    m_pipelineStateDesc._rasterizationState._frontFace = frontFace;
}

void GraphicsPipelineState::setCullMode(CullMode cullMode)
{
    m_pipelineStateDesc._rasterizationState._cullMode = cullMode;
}

void GraphicsPipelineState::setDepthBias(f32 constantFactor, f32 clamp, f32 slopeFactor)
{
    m_pipelineStateDesc._rasterizationState._depthBiasConstant = constantFactor;
    m_pipelineStateDesc._rasterizationState._depthBiasClamp = clamp;
    m_pipelineStateDesc._rasterizationState._depthBiasSlope = slopeFactor;
}

PolygonMode GraphicsPipelineState::getPolygonMode() const
{
    return m_pipelineStateDesc._rasterizationState._polygonMode;
}

FrontFace GraphicsPipelineState::getFrontFace() const
{
    return m_pipelineStateDesc._rasterizationState._frontFace;
}

CullMode GraphicsPipelineState::getCullMode() const
{
    return m_pipelineStateDesc._rasterizationState._cullMode;
}

void GraphicsPipelineState::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
    m_pipelineStateDesc._vertexInputState._primitiveTopology = primitiveTopology;
}

void GraphicsPipelineState::setDepthCompareOp(CompareOperation op)
{
    m_pipelineStateDesc._depthStencilState._compareOp = op;
}

void GraphicsPipelineState::setDepthTest(bool enable)
{
    m_pipelineStateDesc._depthStencilState._depthTestEnable = enable;
}

void GraphicsPipelineState::setDepthWrite(bool enable)
{
    m_pipelineStateDesc._depthStencilState._depthWriteEnable = enable;
}

CompareOperation GraphicsPipelineState::getDepthCompareOp() const
{
    return m_pipelineStateDesc._depthStencilState._compareOp;
}

bool GraphicsPipelineState::isDepthTestEnable() const
{
    return m_pipelineStateDesc._depthStencilState._depthTestEnable;
}

bool GraphicsPipelineState::isDepthWriteEnable() const
{
    return m_pipelineStateDesc._depthStencilState._depthWriteEnable;
}

void GraphicsPipelineState::setBlendEnable(bool enable, s32 attachment)
{
    //TODO attachment
    m_pipelineStateDesc._blendState._colorBlendAttachments._colorBlendEnable = enable;
}

void GraphicsPipelineState::setColorBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment)
{
    //TODO attachment
    m_pipelineStateDesc._blendState._colorBlendAttachments._srcBlendFacor = src;
    m_pipelineStateDesc._blendState._colorBlendAttachments._dscBlendFacor = dst;
}

void GraphicsPipelineState::setColorBlendOp(BlendOperation op, s32 attachment)
{
    //TODO attachment
    m_pipelineStateDesc._blendState._colorBlendAttachments._blendOp = op;
}

void GraphicsPipelineState::setAlphaBlendFactor(BlendFactor src, BlendFactor dst, s32 attachment)
{
    //TODO attachment
    m_pipelineStateDesc._blendState._colorBlendAttachments._srcAlphaBlendFacor = src;
    m_pipelineStateDesc._blendState._colorBlendAttachments._dscAlphaBlendFacor = dst;
}

void GraphicsPipelineState::setAlphaBlendOp(BlendOperation op, s32 attachment)
{
    //TODO attachment
    m_pipelineStateDesc._blendState._colorBlendAttachments._alphaBlendOp = op;
}

bool GraphicsPipelineState::isBlendEnable(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._colorBlendEnable;
}

BlendFactor GraphicsPipelineState::getSrcColorBlendFactor(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._srcBlendFacor;
}

BlendFactor GraphicsPipelineState::getDstColorBlendFactor(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._dscBlendFacor;
}

BlendOperation GraphicsPipelineState::getColorBlendOp(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._blendOp;
}

BlendFactor GraphicsPipelineState::getSrcAlphaBlendFactor(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._srcAlphaBlendFacor;
}

BlendFactor GraphicsPipelineState::getDstAlphaBlendFactor(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._dscAlphaBlendFacor;
}

BlendOperation GraphicsPipelineState::getAlphaBlendOp(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._alphaBlendOp;
}

void GraphicsPipelineState::setColorMask(ColorMaskFlags colormask, s32 attachment)
{
    /*if (attachmet < 0)
    {
        for (auto& attach : m_pipelineStateDesc._blendState._colorBlendAttachments)
        {
            attach._colorWriteMask = colormask;
        }
    }
    else
    {
        m_pipelineStateDesc._blendState._colorBlendAttachments[attachmet]._colorWriteMask = colormask;
    }*/

    m_pipelineStateDesc._blendState._colorBlendAttachments._colorWriteMask = colormask;
}

ColorMaskFlags GraphicsPipelineState::getColorMask(s32 attachment) const
{
    //TODO attachment
    return m_pipelineStateDesc._blendState._colorBlendAttachments._colorWriteMask;
}

PrimitiveTopology GraphicsPipelineState::getPrimitiveTopology() const
{
    return m_pipelineStateDesc._vertexInputState._primitiveTopology;
}

const GraphicsPipelineStateDescription& GraphicsPipelineState::getGraphicsPipelineStateDesc() const
{
    return m_pipelineStateDesc;
}

GraphicsPipelineStateDescription& GraphicsPipelineState::getGraphicsPipelineStateDesc()
{
    return m_pipelineStateDesc;
}

void GraphicsPipelineState::setShaderProgram(const ShaderProgram* program)
{
    m_program = program;
}

void GraphicsPipelineState::setRenderTaget(const RenderTargetState* target)
{
    m_renderTaget = target;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ComputePipelineState::ComputePipelineState(CommandList& cmdList, const ShaderProgram* const program) noexcept
    : PipelineState(cmdList)
    , m_program(program)
{
}

} //renderer
} //v3d
