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

    /*CommandRemoveFramebuffers*/
class CommandRemoveFramebuffers final : public Command
{
public:
    CommandRemoveFramebuffers(const std::vector<Framebuffer*>& framebuffers) noexcept
        : m_framebuffers(framebuffers)
    {
        LOG_DEBUG("CommandRemoveFramebuffers constructor");
    };
    CommandRemoveFramebuffers() = delete;
    CommandRemoveFramebuffers(CommandRemoveFramebuffers&) = delete;

    ~CommandRemoveFramebuffers()
    {
        LOG_DEBUG("CommandRemoveFramebuffers destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandRemoveFramebuffers execute");
        for (auto& framebuffer : m_framebuffers)
        {
            cmdList.getContext()->removeFramebuffer(framebuffer);
        }
    }

private:
    std::vector<Framebuffer*> m_framebuffers;
};

    /*CommandRemoveRenderPasses*/
class CommandRemoveRenderPasses final : public Command
{
public:
    CommandRemoveRenderPasses(const std::vector<RenderPass*>& renderpasses) noexcept
        : m_renderpasses(renderpasses)
    {
        LOG_DEBUG("CommandRemoveRenderPasses constructor");
    };
    CommandRemoveRenderPasses() = delete;
    CommandRemoveRenderPasses(CommandRemoveRenderPasses&) = delete;

    ~CommandRemoveRenderPasses()
    {
        LOG_DEBUG("CommandRemoveRenderPasses destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandRemoveRenderPasses execute");
        for (auto& renderpass : m_renderpasses)
        {
            cmdList.getContext()->removeRenderPass(renderpass);
        }
    }

private:
    std::vector<RenderPass*> m_renderpasses;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept
    : m_cmdList(cmdList)
    , m_size(size)

    , m_trackerRenderpass(this, std::bind(&RenderTarget::destroyRenderPasses, this, std::placeholders::_1))
    , m_trackerFramebuffer(this, std::bind(&RenderTarget::destroyFramebuffers, this, std::placeholders::_1))

{
    std::get<0>(m_depthStencilTexture) = nullptr;
}

RenderTarget::~RenderTarget()
{
    m_trackerFramebuffer.release();
    m_trackerRenderpass.release();
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

bool RenderTarget::setColorTexture(u32 index, BackbufferTexture * swapchainTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D & clearColor)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorattachments, "index >= maxColorattachments");
    if (swapchainTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = swapchainTexture->getFormat();
        attachmentDesc._samples = TextureSamples::TextureSamples_x1;
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;

        m_colorTextures[index] = std::make_tuple(nullptr, attachmentDesc, clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearColor);
    }

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
const core::Dimension2D& RenderTarget::getDimension() const
{
    return m_size;
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

void RenderTarget::extractRenderTargetInfo(RenderPass::RenderPassInfo & renderPassInfo, std::vector<Image*>& images, RenderPass::ClearValueInfo & clearValuesInfo) const
{
    images.reserve(getColorTextureCount() + (hasDepthStencilTexture()) ? 1 : 0);

    clearValuesInfo._size = m_size;
    clearValuesInfo._color.reserve(images.size());

    renderPassInfo._countColorAttachments = getColorTextureCount();
    for (u32 index = 0; index < renderPassInfo._countColorAttachments; ++index)
    {
        auto attachment = m_colorTextures.find(index);
        if (attachment == m_colorTextures.cend())
        {
            continue;
        }

        images.push_back(std::get<0>(attachment->second)->getImage());
        renderPassInfo._attachments[index] = std::get<1>(attachment->second);

        clearValuesInfo._color.push_back(std::get<2>(attachment->second));
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

void RenderTarget::destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers)
{
    if (m_cmdList.isImmediate())
    {
        for (auto& framebuffer : framebuffers)
        {
            m_cmdList.getContext()->removeFramebuffer(framebuffer);
        }
    }
    else
    {
        m_cmdList.pushCommand(new CommandRemoveFramebuffers(framebuffers));
    }
}

void RenderTarget::destroyRenderPasses(const std::vector<RenderPass*>& renderPasses)
{
    if (m_cmdList.isImmediate())
    {
        for (auto& renderPasses : renderPasses)
        {
            m_cmdList.getContext()->removeRenderPass(renderPasses);
        }
    }
    else
    {
        m_cmdList.pushCommand(new CommandRemoveRenderPasses(renderPasses));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Backbuffer::Backbuffer(renderer::CommandList & cmdList, BackbufferTexture * texture) noexcept
    : m_cmdList(cmdList)
    , m_texture(texture)
{
}

} //namespace renderer
} //namespace v3d
