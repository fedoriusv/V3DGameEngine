#include "EmptyContext.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

 EmptyContext::EmptyContext() noexcept
 {
     LOG_DEBUG("EmptyContext created this %llx", this);
     m_renderType = RenderType::EmptyRender;
 }

 EmptyContext::~EmptyContext()
 {
     LOG_DEBUG("~EmptyContext destructor this %llx", this);
 }

void EmptyContext::beginFrame()
{
    LOG_DEBUG("EmptyContext::beginFrame");
}

void EmptyContext::endFrame()
{
    LOG_DEBUG("EmptyContext::endFrame");
}

void EmptyContext::presentFrame()
{
    LOG_DEBUG("EmptyContext::presentFrame");
}

void EmptyContext::submit(bool wait)
{
    LOG_DEBUG("EmptyContext::submit");
}

void EmptyContext::clearBackbuffer(const core::Vector4D & color)
{
    LOG_DEBUG("EmptyContext::clearBackbuffer");
}

void EmptyContext::setViewport(const core::Rect32 & viewport, const core::Vector2D& depth)
{
    LOG_DEBUG("EmptyContext::setViewport");
}

void EmptyContext::setScissor(const core::Rect32 & scissor)
{
    LOG_DEBUG("EmptyContext::setScissor");
}

Image * EmptyContext::createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, TextureUsageFlags flags) const
{
    LOG_DEBUG("EmptyContext::createImage");
    return nullptr;
}

void EmptyContext::removeImage(Image * image)
{
    LOG_DEBUG("EmptyContext::removeImage");
}

void EmptyContext::setRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
    LOG_DEBUG("EmptyContext::setRenderTarget");
}

void EmptyContext::removeFramebuffer(Framebuffer * framebuffer)
{
    LOG_DEBUG("EmptyContext::removeFramebuffer");
}

void EmptyContext::removeRenderPass(RenderPass * renderpass)
{
    LOG_DEBUG("EmptyContext::removeRenderPass");
}

void EmptyContext::invalidateRenderPass()
{
    LOG_DEBUG("EmptyContext::invalidateRenderPass");
}

void EmptyContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    LOG_DEBUG("EmptyContext::setPipeline");
}

void EmptyContext::removePipeline(Pipeline * pipeline)
{
    LOG_DEBUG("EmptyContext::removePipeline");
}

void EmptyContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
    LOG_DEBUG("EmptyContext::bindUniformsBuffer");
}

//void EmptyContext::bindVertexBuffers(const std::vector<Buffer*>& buffer, const std::vector<u64>& offsets)
//{
//    LOG_DEBUG("EmptyContext::bindVertexBuffers");
//}

void EmptyContext::draw(StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    LOG_DEBUG("EmptyContext::draw");
}

void EmptyContext::drawIndexed(StreamBufferDescription & desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
    LOG_DEBUG("EmptyContext::drawIndexed");
}

void EmptyContext::bindImage(const Shader * shader, u32 bindIndex, const Image * image)
{
    LOG_DEBUG("EmptyContext::bindImage");
}

void EmptyContext::bindSampler(const Shader * shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
    LOG_DEBUG("EmptyContext::bindSampler");
}

void EmptyContext::bindSampledImage(const Shader * shader, u32 bindIndex, const Image * image, const Sampler::SamplerInfo* samplerInfo)
{
    LOG_DEBUG("EmptyContext::bindSampledImage");
}

const DeviceCaps* EmptyContext::getDeviceCaps() const
{
    return nullptr;
}

Framebuffer * EmptyContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D & size)
{
    LOG_DEBUG("EmptyContext::createFramebuffer");
    return nullptr;
}

RenderPass * EmptyContext::createRenderPass(const RenderPassDescription * renderpassDesc)
{
    LOG_DEBUG("EmptyContext::createRenderPass");
    return nullptr;
}

Pipeline * EmptyContext::createPipeline(Pipeline::PipelineType type)
{
    LOG_DEBUG("EmptyContext::createPipeline");
    return nullptr;
}

Sampler * EmptyContext::createSampler()
{
    LOG_DEBUG("EmptyContext::createSampler");
    return nullptr;
}

Buffer * EmptyContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size)
{
    LOG_DEBUG("EmptyContext::createBuffer");
    return nullptr;
}

void EmptyContext::removeBuffer(Buffer * buffer)
{
    LOG_DEBUG("EmptyContext::removeBuffer");
}

void EmptyContext::removeSampler(Sampler * sampler)
{
    LOG_DEBUG("EmptyContext::removeSampler");
}

bool EmptyContext::initialize()
{
    LOG_DEBUG("EmptyContext::initialize");
    return true;
}

void EmptyContext::destroy()
{
    LOG_DEBUG("EmptyContext::destroy");
}

} //namespace renderer
} //namespace v3d
