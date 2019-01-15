#include "RenderTarget.h"
#include "Texture.h"

#include "Renderer/Context.h"
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

    /*CommandRemoveRenderTarget*/
class CommandRemoveRenderTarget final : public Command
{
public:
    CommandRemoveRenderTarget(const RenderPass::RenderPassInfo& renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo& clearInfo) noexcept
        : m_renderpassInfo(renderpassInfo)
        , m_attachments(attachments)
        , m_clearInfo(clearInfo)
    {
        LOG_DEBUG("CommandRemoveRenderTarget constructor");
    };
    CommandRemoveRenderTarget() = delete;
    CommandRemoveRenderTarget(CommandRemoveRenderTarget&) = delete;

    ~CommandRemoveRenderTarget()
    {
        LOG_DEBUG("CommandRemoveRenderTarget destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->removeRenderTarget(&m_renderpassInfo, m_attachments, &m_clearInfo);
    }

private:
    RenderPass::RenderPassInfo  m_renderpassInfo;
    std::vector<Image*>         m_attachments;
    RenderPass::ClearValueInfo  m_clearInfo;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept
    : m_cmdList(cmdList)
    , m_size(size)
{
    std::get<0>(m_depthStencilTexture) = nullptr;
}

RenderTarget::~RenderTarget()
{
    RenderTarget::destroy();
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

void RenderTarget::extractRenderTargetInfo(RenderPass::RenderPassInfo & renderPassInfo, std::vector<Image*>& images, RenderPass::ClearValueInfo & clearValuesInfo)
{
    images.reserve(getColorTextureCount() + (hasDepthStencilTexture()) ? 1 : 0);

    clearValuesInfo._size = m_size;
    clearValuesInfo._color.reserve(images.size());

    renderPassInfo._countColorAttachments = getColorTextureCount();
    for (u32 index = 0; index < renderPassInfo._countColorAttachments; ++index)
    {
        auto attachment = m_colorTextures[index];

        images.push_back(std::get<0>(attachment)->getImage());
        renderPassInfo._attachments[index] = std::get<1>(attachment);

        clearValuesInfo._color.push_back(std::get<2>(attachment));
    }

    renderPassInfo._hasDepthStencilAttahment = hasDepthStencilTexture();
    if (renderPassInfo._hasDepthStencilAttahment)
    {
        images.push_back(getDepthStencilTexture()->getImage());
        renderPassInfo._attachments.back() = std::get<1>(m_depthStencilTexture);

        clearValuesInfo._depth = std::get<2>(m_depthStencilTexture);
        clearValuesInfo._stencil = std::get<3>(m_depthStencilTexture);
    }
}

void RenderTarget::destroy()
{
    RenderPass::RenderPassInfo renderPassInfo;
    std::vector<renderer::Image*> images;
    RenderPass::ClearValueInfo clearValuesInfo;
    RenderTarget::extractRenderTargetInfo(renderPassInfo, images, clearValuesInfo);

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeRenderTarget(&renderPassInfo, images, &clearValuesInfo);
    }
    else
    {
        m_cmdList.flushPendingCommands(CommandList::PendingFlush_UpdateRenderTarget);
        m_cmdList.pushCommand(new CommandRemoveRenderTarget(renderPassInfo, images, clearValuesInfo));
    }
}

Backbuffer::Backbuffer(renderer::CommandList & cmdList, SwapchainTexture * texture) noexcept
    : m_cmdList(cmdList)
    , m_texture(texture)
{
}

} //namespace renderer
} //namespace v3d
