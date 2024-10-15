#include "PipelineState.h"

namespace v3d
{
namespace renderer
{

GraphicsPipelineState::GraphicsPipelineState(Device* device, const VertexInputAttributeDesc& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
    m_pipelineStateDesc._vertexInputState._inputAttributes = vertex;
}

GraphicsPipelineState::GraphicsPipelineState(Device* device, const GraphicsPipelineStateDesc& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_pipelineStateDesc(desc)
    , m_program(program)
    , m_renderTaget(renderTaget)
{
}

} //namespace renderer
} //namespace v3d