#include "PipelineState.h"
#include "Renderer/Context.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CommandRemovePipelines*/
class CommandRemovePipelines final : public Command
{
public:
    CommandRemovePipelines(const std::vector<Pipeline*>& pipelines) noexcept
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const VertexInputAttribDescription& vertex, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept
    : m_cmdList(cmdList)
    , m_program(program)
    , m_renderTaget(renderTaget)

    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
    m_pipelineStateDesc._vertexInputState._inputAttributes = vertex;
}

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateDescription& desc, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept
    : m_cmdList(cmdList)
    , m_pipelineStateDesc(desc)
    , m_program(program)
    , m_renderTaget(renderTaget)

    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
}

GraphicsPipelineState::~GraphicsPipelineState()
{
    m_tracker.release();
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

const GraphicsPipelineStateDescription & GraphicsPipelineState::getGraphicsPipelineStateDesc() const
{
    return m_pipelineStateDesc;
}

GraphicsPipelineStateDescription& GraphicsPipelineState::getGraphicsPipelineStateDesc()
{
    return m_pipelineStateDesc;
}

void GraphicsPipelineState::setShaderProgram(const ShaderProgram * program)
{
    m_program = program;
}

void GraphicsPipelineState::setRenderTaget(const RenderTarget * target)
{
    m_renderTaget = target;
}

void GraphicsPipelineState::destroyPipelines(const std::vector<Pipeline*>& pipelines)
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
        m_cmdList.pushCommand(new CommandRemovePipelines(pipelines));
    }
}

} //renderer
} //v3d
