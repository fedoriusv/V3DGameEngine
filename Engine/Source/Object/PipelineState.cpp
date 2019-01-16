#include "PipelineState.h"

namespace v3d
{
namespace renderer
{

GraphicsPipelineState::RasterizationState::RasterizationState()
    : _polygonMode(PolygonMode::PolygonMode_Triangle)
    , _frontFace(FrontFace::FrontFace_Clockwise)
    , _cullMode(CullMode::CullMode_Back)
{
}

GraphicsPipelineState::GraphicsPipelineStateInfo::GraphicsPipelineStateInfo()
    : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
{
}

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept
    : m_cmdList(cmdList)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
}

GraphicsPipelineState::GraphicsPipelineState(CommandList& cmdList, const GraphicsPipelineStateInfo& desc, const ShaderProgram* const program, const RenderTarget* const renderTaget) noexcept
    : m_cmdList(cmdList)
    , m_pipelineStateDesc(desc)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
}

GraphicsPipelineState::~GraphicsPipelineState()
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

void GraphicsPipelineState::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
    m_pipelineStateDesc._primitiveTopology = primitiveTopology;
}

const GraphicsPipelineState::GraphicsPipelineStateInfo & GraphicsPipelineState::getGraphicsPipelineStateDesc() const
{
    return m_pipelineStateDesc;
}

GraphicsPipelineState::GraphicsPipelineStateInfo& GraphicsPipelineState::getGraphicsPipelineStateDesc()
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

} //renderer
} //v3d
