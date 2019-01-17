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

void EmptyContext::clearBackbuffer(const core::Vector4D & color)
{
    LOG_DEBUG("EmptyContext::clearBackbuffer");
}

void EmptyContext::setViewport(const core::Rect32 & viewport)
{
    LOG_DEBUG("EmptyContext::setViewport");
}

Image * EmptyContext::createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel,
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    LOG_DEBUG("EmptyContext::createImage");
    return nullptr;
}

Image * EmptyContext::createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples,
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    LOG_DEBUG("EmptyContext::createAttachmentImage");
    return nullptr;
}

void EmptyContext::setRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo, 
    const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers)
{
    LOG_DEBUG("EmptyContext::setRenderTarget");
}

void EmptyContext::removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo)
{
    LOG_DEBUG("EmptyContext::removeRenderTarget");
}

void EmptyContext::removeFramebuffer(Framebuffer * framebuffer)
{
    LOG_DEBUG("EmptyContext::removeFramebuffer");
}

void EmptyContext::removeRenderPass(RenderPass * renderpass)
{
    LOG_DEBUG("EmptyContext::removeRenderPass");
}

void EmptyContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo, ObjectTracker<Pipeline>* tracker)
{
    LOG_DEBUG("EmptyContext::setPipeline");
}

void EmptyContext::removePipeline(Pipeline * pipeline)
{
    LOG_DEBUG("EmptyContext::removePipeline");
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

RenderPass * EmptyContext::createRenderPass(const RenderPass::RenderPassInfo * renderpassInfo)
{
    LOG_DEBUG("EmptyContext::createRenderPass");
    return nullptr;
}

Pipeline * EmptyContext::createPipeline(Pipeline::PipelineType type)
{
    LOG_DEBUG("EmptyContext::createPipeline");
    return nullptr;
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
