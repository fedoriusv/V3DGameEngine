#include "PipelineState.h"
#include "Device.h"
#include "Stream/Stream.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

 u32 VertexInputAttributeDesc::operator>>(stream::Stream* stream) const
 {
     u32 writeSize = 0;
     u32 writePos = stream->tell();

     stream->write<u32>(_countInputBindings);
     writeSize += sizeof(u32);
     for (u32 i = 0; i < _countInputBindings; ++i)
     {
         stream->write<InputBinding>(_inputBindings[i]);
         writeSize += sizeof(u32);
     }

     stream->write<u32>(_countInputAttributes);
     writeSize += sizeof(u32);
     for (u32 i = 0; i < _countInputAttributes; ++i)
     {
         stream->write<InputAttribute>(_inputAttributes[i]);
         writeSize += sizeof(u32);
     }

     writePos = stream->tell() - writePos;
     ASSERT(writePos == writeSize, "wrong size");
     return writeSize;
 }

 u32 VertexInputAttributeDesc::operator<<(const stream::Stream* stream)
 {
     u32 readSize = 0;
     u32 readPos = stream->tell();

     u32 countInputBindings;
     stream->read<u32>(countInputBindings);
     readSize += sizeof(u32);
     for (u32 i = 0; i < countInputBindings; ++i)
     {
         stream->read<InputBinding>(_inputBindings[i]);
         readSize += sizeof(u32);
     }
     _countInputBindings = countInputBindings;

     u32 countInputAttributes;
     stream->read<u32>(countInputAttributes);
     readSize += sizeof(u32);
     for (u32 i = 0; i < countInputAttributes; ++i)
     {
         stream->read<InputAttribute>(_inputAttributes[i]);
         readSize += sizeof(u32);
     }
     _countInputAttributes = countInputAttributes;

     readPos = stream->tell() - readPos;
     ASSERT(readPos == readSize, "wrong size");
     return readSize;
 }

GraphicsPipelineState::GraphicsPipelineState(Device* device, const VertexInputAttributeDesc& attributeDesc, const RenderPassDesc& renderpassDesc, const ShaderProgram* const program, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_program(program)
    , m_renderPassDesc(renderpassDesc)
    , m_name(name)

    , m_tracker(this, std::bind(&GraphicsPipelineState::destroyPipelines, this, std::placeholders::_1))
{
    LOG_DEBUG("GraphicsPipelineState::GraphicsPipelineState constructor %llx", this);

    m_pipelineStateDesc._vertexInputState._inputAttributes = attributeDesc;
}

GraphicsPipelineState::GraphicsPipelineState(Device* device, const GraphicsPipelineStateDesc& pipelineDesc, const RenderPassDesc& renderpassDesc, const ShaderProgram* const program, const std::string& name) noexcept
    : PipelineState()
    , m_device(device)
    , m_pipelineStateDesc(pipelineDesc)
    , m_renderPassDesc(renderpassDesc)
    , m_program(program)
    , m_name(name)

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