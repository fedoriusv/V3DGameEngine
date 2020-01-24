#include "RenderTargetState.h"
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
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveFramebuffers constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandRemoveFramebuffers() = delete;
    CommandRemoveFramebuffers(CommandRemoveFramebuffers&) = delete;

    ~CommandRemoveFramebuffers()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveFramebuffers destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveFramebuffers execute");
#endif //DEBUG_COMMAND_LIST
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
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveRenderPasses constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandRemoveRenderPasses() = delete;
    CommandRemoveRenderPasses(CommandRemoveRenderPasses&) = delete;

    ~CommandRemoveRenderPasses()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveRenderPasses destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandRemoveRenderPasses execute");
#endif //DEBUG_COMMAND_LIST
        for (auto& renderpass : m_renderpasses)
        {
            cmdList.getContext()->removeRenderPass(renderpass);
        }
    }

private:
    std::vector<RenderPass*> m_renderpasses;
};

/*CommandClearBackbuffer*/
class CommandClearBackbuffer : public renderer::Command
{
public:
    CommandClearBackbuffer(const core::Vector4D& color) noexcept
        : m_clearColor(color)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer constructor");
#endif //DEBUG_COMMAND_LIST
    };

    ~CommandClearBackbuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const renderer::CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->clearBackbuffer(m_clearColor);
    }

private:
    core::Vector4D m_clearColor;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTargetState::RenderTargetState(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept
    : m_cmdList(cmdList)
    , m_size(size)

    , m_trackerFramebuffer(this, std::bind(&RenderTargetState::destroyFramebuffers, this, std::placeholders::_1))
    , m_trackerRenderpass(this, std::bind(&RenderTargetState::destroyRenderPasses, this, std::placeholders::_1))

{
    std::get<0>(m_depthStencilTexture) = nullptr;
}

RenderTargetState::~RenderTargetState()
{
    m_trackerRenderpass.release();
    m_trackerFramebuffer.release();
}

bool RenderTargetState::setColorTexture(u32 index, Texture2D* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorAttachments, "index >= maxColorattachments");
    if (colorTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSampleCount();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._internalTarget = false;
        attachmentDesc._autoResolve = colorTexture->isTextureUsageFlagsContains(TextureUsage::TextureUsage_Resolve);

        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_ColorAttachmet;

        m_colorTextures[index] = std::make_tuple(colorTexture, attachmentDesc, clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearColor);
    }

    //TODO check compatibility
    return true;
}

bool RenderTargetState::setColorTexture(u32 index, Backbuffer* swapchainTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D & clearColor)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorAttachments, "index >= maxColorAttachments");
    if (swapchainTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = swapchainTexture->getFormat();
        attachmentDesc._samples = TextureSamples::TextureSamples_x1;
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._internalTarget = true;
        attachmentDesc._autoResolve = false;

        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_Present;

        m_colorTextures[index] = std::make_tuple(nullptr, attachmentDesc, clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearColor);
    }

    return true;
}

bool RenderTargetState::setDepthStencilTexture(Texture2D* depthStencilTexture, 
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
        attachmentDesc._internalTarget = false;
        attachmentDesc._autoResolve = depthStencilTexture->isTextureUsageFlagsContains(TextureUsage::TextureUsage_Resolve);

        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachmet;

        m_depthStencilTexture = std::make_tuple(depthStencilTexture, attachmentDesc, clearDepth, clearStencil);
    }
    else
    {
        m_depthStencilTexture = std::make_tuple(nullptr, renderer::AttachmentDescription(), clearDepth, clearStencil);
    }

    //TODO check compatibility
    return true;
}

bool RenderTargetState::setColorTexture(u32 index, Texture2D * colorTexture, const ColorOpState & colorOpState, const TransitionState & tansitionState)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorAttachments, "index >= maxColorAttachments");
    if (colorTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSampleCount();
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._internalTarget = false;
        attachmentDesc._autoResolve = colorTexture->isTextureUsageFlagsContains(TextureUsage::TextureUsage_Resolve);

        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;

        m_colorTextures[index] = std::make_tuple(colorTexture, attachmentDesc, colorOpState._clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), colorOpState._clearColor);
    }

    return true;
}

bool RenderTargetState::setColorTexture(u32 index, Backbuffer * swapchainTexture, const ColorOpState & colorOpState, const TransitionState & tansitionState)
{
    ASSERT(index < m_cmdList.getContext()->getDeviceCaps()->maxColorAttachments, "index >= maxColorAttachments");
    if (swapchainTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = swapchainTexture->getFormat();
        attachmentDesc._samples = TextureSamples::TextureSamples_x1;
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._internalTarget = true;
        attachmentDesc._autoResolve = false;

        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;

        m_colorTextures[index] = std::make_tuple(nullptr, attachmentDesc, colorOpState._clearColor);
    }
    else
    {
        m_colorTextures[index] = std::make_tuple(nullptr, renderer::AttachmentDescription(), colorOpState._clearColor);
    }

    return true;
}

bool RenderTargetState::setDepthStencilTexture(Texture2D * depthStencilTexture, const DepthOpState & depthOpState, const StencilOpState & stencilOpState, const TransitionState & tansitionState)
{
    if (depthStencilTexture)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSampleCount();
        attachmentDesc._loadOp = depthOpState._loadOp;
        attachmentDesc._storeOp = depthOpState._storeOp;
        attachmentDesc._stencilLoadOp = stencilOpState._loadOp;
        attachmentDesc._stencilStoreOp = stencilOpState._storeOp;
        attachmentDesc._internalTarget = false;
        attachmentDesc._autoResolve = depthStencilTexture->isTextureUsageFlagsContains(TextureUsage::TextureUsage_Resolve);

        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;

        m_depthStencilTexture = std::make_tuple(depthStencilTexture, attachmentDesc, depthOpState._clearDepth, stencilOpState._clearStencil);
    }
    else
    {
        m_depthStencilTexture = std::make_tuple(nullptr, renderer::AttachmentDescription(), depthOpState._clearDepth, stencilOpState._clearStencil);
    }

    return true;
}

Texture2D * RenderTargetState::getColorTexture(u32 index) const
{
    ASSERT(index < m_colorTextures.size(), "out of range");
    auto attachment = m_colorTextures.find(index);
    if (attachment == m_colorTextures.cend())
    {
        return nullptr;
    }

    return std::get<0>((*attachment).second);
}
const core::Dimension2D& RenderTargetState::getDimension() const
{
    return m_size;
}


Texture2D * RenderTargetState::getDepthStencilTexture() const
{
    return std::get<0>(m_depthStencilTexture);
}

u32 RenderTargetState::getColorTextureCount() const
{
    return static_cast<u32>(m_colorTextures.size());
}

bool RenderTargetState::hasDepthStencilTexture() const
{
    return std::get<0>(m_depthStencilTexture) != nullptr;
}

void RenderTargetState::extractRenderTargetInfo(RenderPassDescription& renderpassDesc, std::vector<Image*>& images, Framebuffer::ClearValueInfo& clearValuesInfo) const
{
    images.reserve(static_cast<u64>(getColorTextureCount()) + (hasDepthStencilTexture() ? 1 : 0));

    clearValuesInfo._size = m_size;
    clearValuesInfo._color.reserve(images.size());

    renderpassDesc._countColorAttachments = getColorTextureCount();
    for (u32 index = 0; index < renderpassDesc._countColorAttachments; ++index)
    {
        auto attachment = m_colorTextures.find(index);
        if (attachment == m_colorTextures.cend())
        {
            continue;
        }

        if (std::get<1>(attachment->second)._internalTarget)
        {
            images.push_back(nullptr);
        }
        else
        {
            images.push_back(std::get<0>(attachment->second)->getImage());
        }
        renderpassDesc._attachments[index] = std::get<1>(attachment->second);
        clearValuesInfo._color.push_back(std::get<2>(attachment->second));
    }

    renderpassDesc._hasDepthStencilAttahment = hasDepthStencilTexture();
    if (renderpassDesc._hasDepthStencilAttahment)
    {
        images.push_back(getDepthStencilTexture()->getImage());
        renderpassDesc._attachments.back() = std::get<1>(m_depthStencilTexture);

        clearValuesInfo._depth = std::get<2>(m_depthStencilTexture);
        clearValuesInfo._stencil = std::get<3>(m_depthStencilTexture);
    }
}

void RenderTargetState::destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers)
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

void RenderTargetState::destroyRenderPasses(const std::vector<RenderPass*>& renderPasses)
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

Backbuffer::Backbuffer(renderer::CommandList& cmdList) noexcept
    : m_cmdList(cmdList)
{
}

Backbuffer::~Backbuffer()
{
}

const core::Dimension2D & Backbuffer::getDimension() const
{
    return m_cmdList.getContext()->m_backufferDescription._size;
}

renderer::Format Backbuffer::getFormat() const
{
    return m_cmdList.getContext()->m_backufferDescription._format;
}

void Backbuffer::read(const core::Dimension2D & offset, const core::Dimension2D & size, void * const data)
{
}

void Backbuffer::clear(const core::Vector4D & color)
{
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->clearBackbuffer(color);
    }
    else
    {
        m_cmdList.pushCommand(new CommandClearBackbuffer(color));
    }
}

} //namespace renderer
} //namespace v3d
