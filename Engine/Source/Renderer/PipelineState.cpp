#include "PipelineState.h"
#include "Device.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

GraphicsPipelineState::GraphicsPipelineState(Device* device, const VertexInputAttributeDesc& vertex, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_program(program)
    , m_renderTaget(renderTaget)

    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
    LOG_DEBUG("GraphicsPipelineState::GraphicsPipelineState constructor %llx", this);

    m_pipelineStateDesc._vertexInputState._inputAttributes = vertex;
}

GraphicsPipelineState::GraphicsPipelineState(Device* device, const GraphicsPipelineStateDesc& desc, const ShaderProgram* const program, const RenderTargetState* const renderTaget, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_pipelineStateDesc(desc)
    , m_program(program)
    , m_renderTaget(renderTaget)

    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
    LOG_DEBUG("GraphicsPipelineState::GraphicsPipelineState constructor %llx", this);
}

GraphicsPipelineState::~GraphicsPipelineState()
{
    LOG_DEBUG("GraphicsPipelineState::GraphicsPipelineState destructor %llx", this);
    m_tracker.release();
}

void GraphicsPipelineState::destroyPipelines(const std::vector<RenderPipeline*>& pipelines)
{
    for (auto pipeline : pipelines)
    {
        m_device->destroyPipeline(pipeline);
    }
}


ComputePipelineState::ComputePipelineState(Device* device, const ShaderProgram* program, const std::string& name) noexcept
    : m_device(*device)
    , m_program(program)
    , m_name(name)

    , m_tracker(this, std::bind(&ComputePipelineState::destroyPipelines, this, std::placeholders::_1))
{
    LOG_DEBUG("ComputePipelineState::ComputePipelineState constructor %llx", this);
}

ComputePipelineState::~ComputePipelineState()
{
    LOG_DEBUG("ComputePipelineState::ComputePipelineState destructor %llx", this);
    m_tracker.release();
}

void ComputePipelineState::destroyPipelines(const std::vector<RenderPipeline*>& pipelines)
{
    for (auto pipeline : pipelines)
    {
        m_device.destroyPipeline(pipeline);
    }
}

} //namespace renderer
} //namespace v3d