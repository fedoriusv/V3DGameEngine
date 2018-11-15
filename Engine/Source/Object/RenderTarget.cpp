#include "RenderTarget.h"
#include "Context.h"
#include "Texture.h"

#include "Renderer/Image.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/DeviceCaps.h"

#include "Utils/Logger.h"


namespace v3d
{
namespace renderer
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size)
    : m_cmdList(cmdList)
    , m_size(size)
{
    std::get<0>(m_depthStencilTexture) = nullptr;
}

RenderTarget::~RenderTarget()
{
}

bool RenderTarget::setColorTexture(u32 index, Texture2D* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorattachments, "index >= maxColorattachments");

    if (colorTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSampleCount();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;

        m_colorTextures[index] = std::make_tuple(colorTexture, attachmentDesc, clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearColor);
    }

    //TODO check compatibility
    return true;
}

bool RenderTarget::setDepthStencilTexture(Texture2D* depthStencilTexture, 
    RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
{
    if (depthStencilTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSampleCount();
        attachmentDesc._loadOp = depthLoadOp;
        attachmentDesc._storeOp = depthStoreOp;
        attachmentDesc._stencilLoadOp = stencilLoadOp;
        attachmentDesc._stencilStoreOp = stencilStoreOp;
        m_depthStencilTexture = std::make_tuple(depthStencilTexture, attachmentDesc, clearDepth, clearStencil);
    }
    else
    {
        m_depthStencilTexture = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearDepth, clearStencil);
    }

    //TODO check compatibility
    return true;
}

Texture2D * RenderTarget::getColorTexture(u32 index) const
{
    ASSERT(index < m_colorTextures.size(), "out of range");
    auto attachment = m_colorTextures.find(index);
    if (attachment == m_colorTextures.cend())
    {
        return nullptr;
    }

    return std::get<0>((*attachment).second);
}

Texture2D * RenderTarget::getDepthStencilTexture() const
{
    return std::get<0>(m_depthStencilTexture);
}

u32 RenderTarget::getColorTextureCount() const
{
    return static_cast<u32>(m_colorTextures.size());
}

bool RenderTarget::hasDepthStencilTexture() const
{
    return std::get<0>(m_depthStencilTexture) != nullptr;
}

Backbuffer::Backbuffer(renderer::CommandList & cmdList, SwapchainTexture * texture)
    : m_cmdList(cmdList)
    , m_texture(texture)
{
}

} //namespace renderer
} //namespace v3d
