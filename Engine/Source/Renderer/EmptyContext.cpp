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

Image * EmptyContext::createImage(TextureTarget target, renderer::ImageFormat format, const core::Dimension3D& dimension, u32 mipmapLevel,
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    LOG_DEBUG("EmptyContext::createImage");
    return nullptr;
}

Image * EmptyContext::createAttachmentImage(renderer::ImageFormat format, const core::Dimension3D& dimension, TextureSamples samples,
    s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const
{
    LOG_DEBUG("EmptyContext::createAttachmentImage");
    return nullptr;
}

void EmptyContext::setRenderTarget(const RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const ClearValueInfo * clearInfo)
{
    LOG_DEBUG("EmptyContext::setRenderTarget");
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

RenderPass * EmptyContext::createRenderPass(const RenderPassInfo * renderpassInfo)
{
    LOG_DEBUG("EmptyContext::createRenderPass");
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
