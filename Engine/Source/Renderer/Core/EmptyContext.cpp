#include "EmptyContext.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

 EmptyContext::EmptyContext() noexcept
 {
     LOG_DEBUG("EmptyContext::EmptyContext created this %llx", this);
     m_renderType = RenderType::EmptyRender;
 }

 EmptyContext::~EmptyContext()
 {
     LOG_DEBUG("EmptyContext::~EmptyContext destructor this %llx", this);
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

void EmptyContext::beginQuery(const Query* query, const std::string& tag)
{
    LOG_DEBUG("EmptyContext::beginQuery");
}

void EmptyContext::endQuery(const Query* query, const std::string& tag)
{
    LOG_DEBUG("EmptyContext::endQuery");
}

void EmptyContext::timestampQuery(const Query* query, const std::string& tag)
{
    LOG_DEBUG("EmptyContext::timestampQuery");
}

void EmptyContext::clearBackbuffer(const core::Vector4D& color)
{
    LOG_DEBUG("EmptyContext::clearBackbuffer");
}

void EmptyContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
    LOG_DEBUG("EmptyContext::setViewport");
}

void EmptyContext::setScissor(const core::Rect32& scissor)
{
    LOG_DEBUG("EmptyContext::setScissor");
}

Image* EmptyContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, const std::string& name)
{
    LOG_DEBUG("EmptyContext::createImage");
    return nullptr;
}

Image* EmptyContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name)
{
    LOG_DEBUG("EmptyContext::createImage");
    return nullptr;
}

void EmptyContext::removeImage(Image* image)
{
    LOG_DEBUG("EmptyContext::removeImage");
}

void EmptyContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
    LOG_DEBUG("EmptyContext::setRenderTarget");
}

void EmptyContext::removeFramebuffer(Framebuffer* framebuffer)
{
    LOG_DEBUG("EmptyContext::removeFramebuffer");
}

void EmptyContext::removeRenderPass(RenderPass* renderpass)
{
    LOG_DEBUG("EmptyContext::removeRenderPass");
}

Query* EmptyContext::createQuery(QueryType type, const Query::QueryRespose& callback, const std::string& name)
{
    LOG_DEBUG("EmptyContext::createQuery");
    return nullptr;
}

void EmptyContext::removeQuery(Query* query)
{
    LOG_DEBUG("EmptyContext::removeQuery");
}

void EmptyContext::invalidateRenderTarget()
{
    LOG_DEBUG("EmptyContext::invalidateRenderTarget");
}

void EmptyContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    LOG_DEBUG("EmptyContext::setPipeline");
}

void EmptyContext::setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo)
{
    LOG_DEBUG("EmptyContext::setPipeline");
}

void EmptyContext::removePipeline(Pipeline* pipeline)
{
    LOG_DEBUG("EmptyContext::removePipeline");
}

void EmptyContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
    LOG_DEBUG("EmptyContext::bindUniformsBuffer");
}

void EmptyContext::bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
    LOG_DEBUG("EmptyContext::bindStorageImage");
}

void EmptyContext::transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition)
{
    LOG_DEBUG("EmptyContext::transitionImages");
}

void EmptyContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
    LOG_DEBUG("EmptyContext::draw");
}

void EmptyContext::drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
    LOG_DEBUG("EmptyContext::drawIndexed");
}

void EmptyContext::dispatchCompute(const core::Dimension3D& groups)
{
    LOG_DEBUG("EmptyContext::dispatchCompute");
}

void EmptyContext::bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
    LOG_DEBUG("EmptyContext::bindImage");
}

void EmptyContext::bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
    LOG_DEBUG("EmptyContext::bindSampler");
}

void EmptyContext::bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer, s32 mip)
{
    LOG_DEBUG("EmptyContext::bindSampledImage");
}

const DeviceCaps* EmptyContext::getDeviceCaps() const
{
    return nullptr;
}

Framebuffer* EmptyContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size)
{
    LOG_DEBUG("EmptyContext::createFramebuffer");
    return nullptr;
}

RenderPass* EmptyContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
    LOG_DEBUG("EmptyContext::createRenderPass");
    return nullptr;
}

Pipeline* EmptyContext::createPipeline(Pipeline::PipelineType type)
{
    LOG_DEBUG("EmptyContext::createPipeline");
    return nullptr;
}

Sampler* EmptyContext::createSampler(const SamplerDescription& desc)
{
    LOG_DEBUG("EmptyContext::createSampler");
    return nullptr;
}

Buffer* EmptyContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, const std::string& name)
{
    LOG_DEBUG("EmptyContext::createBuffer");
    return nullptr;
}

void EmptyContext::removeBuffer(Buffer* buffer)
{
    LOG_DEBUG("EmptyContext::removeBuffer");
}

void EmptyContext::removeSampler(Sampler* sampler)
{
    LOG_DEBUG("EmptyContext::removeSampler");
}

void EmptyContext::generateMipmaps(Image* image, u32 layer, TransitionOp state)
{
    LOG_DEBUG("EmptyContext::generateMipmaps");
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
