#include "PipelineState.h"

namespace v3d
{
namespace renderer
{

GraphicsPipelineState::GraphicsPipelineState() noexcept
{
}

GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc & desc) noexcept
    : m_pipelineStateDesc(desc)
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

GraphicsPipelineState::RasterizationState::RasterizationState()
    : _polygonMode(PolygonMode::PolygonMode_Triangle)
    , _frontFace(FrontFace::FrontFace_Clockwise)
    , _cullMode(CullMode::CullMode_Back)
{
}

GraphicsPipelineState::GraphicsPipelineStateDesc::GraphicsPipelineStateDesc()
    : _primitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList)
{
}

} //renderer
} //v3d
