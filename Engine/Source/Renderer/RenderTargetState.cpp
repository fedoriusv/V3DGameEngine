#include "RenderTargetState.h"
#include "Device.h"
#include "DeviceCaps.h"
#include "Texture.h"
#include "Utils/Logger.h"


namespace v3d
{
namespace renderer
{

RenderTargetState::RenderTargetState(Device* device, const math::Dimension2D& size, u32 countAttacment, u32 viewsMask, const std::string& name) noexcept
    : m_device(device)
    , m_name(name)

    , m_trackerFramebuffer(this, std::bind(&RenderTargetState::destroyFramebuffers, this, std::placeholders::_1))
    , m_trackerRenderpass(this, std::bind(&RenderTargetState::destroyRenderPasses, this, std::placeholders::_1))
{
    LOG_DEBUG("RenderTargetState::RenderTargetState constructor %llx", this);

    ASSERT(countAttacment < m_device->getDeviceCaps()._maxColorAttachments, "index >= maxColorattachments");
    m_renderpassDesc._countColorAttachment = countAttacment;
    m_attachmentsDesc._renderArea = size;
    m_attachmentsDesc._viewsMask = viewsMask;
}

RenderTargetState::~RenderTargetState()
{
    LOG_DEBUG("RenderTargetState::RenderTargetState destructor %llx", this);
    m_trackerFramebuffer.release();
    m_trackerRenderpass.release();
}

bool RenderTargetState::setColorTexture(u32 index, const TextureView& colorTextureView, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const color::Color& clearColor)
{
    ASSERT(index < m_renderpassDesc._countColorAttachment, "index >= count of attachments");
    if (colorTextureView._texture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTextureView._texture->getFormat();
        attachmentDesc._samples = colorTextureView._texture->getSamples();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._transition = TransitionOp::TransitionOp_ColorAttachment;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
        m_attachmentsDesc._imageViews[index] = colorTextureView;
        m_attachmentsDesc._clearColorValues[index] = clearColor;

        return checkCompatibility(colorTextureView, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._imageViews[index] = TextureView();
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture(const TextureView& depthStencilTextureView, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
{
    if (depthStencilTextureView._texture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTextureView._texture->getFormat();
        attachmentDesc._samples = depthStencilTextureView._texture->getSamples();
        attachmentDesc._loadOp = depthLoadOp;
        attachmentDesc._storeOp = depthStoreOp;
        attachmentDesc._stencilLoadOp = stencilLoadOp;
        attachmentDesc._stencilStoreOp = stencilStoreOp;
        attachmentDesc._transition = TransitionOp::TransitionOp_DepthStencilAttachment;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
        m_attachmentsDesc._imageViews.back() = depthStencilTextureView;
        m_attachmentsDesc._clearDepthValue = clearDepth;
        m_attachmentsDesc._clearStencilValue = clearStencil;
        m_renderpassDesc._hasDepthStencilAttachment = true;

        return checkCompatibility(depthStencilTextureView, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._imageViews.back() = TextureView();

        m_renderpassDesc._hasDepthStencilAttachment = false;
    }

    return false;
}

bool RenderTargetState::setColorTexture(u32 index, const TextureView& colorTextureView, const ColorOpState& colorOpState, const TransitionState& tansitionState)
{
    ASSERT(index < m_renderpassDesc._countColorAttachment, "index >= count of attachments");
    if (colorTextureView._texture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTextureView._texture->getFormat();
        attachmentDesc._samples = colorTextureView._texture->getSamples();
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._transition = tansitionState._state;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._imageViews[index] = colorTextureView;
        m_attachmentsDesc._clearColorValues[index] = colorOpState._clearColor;

        return checkCompatibility(colorTextureView, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._imageViews[index] = TextureView();
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture(const TextureView& depthStencilTextureView, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState)
{
    if (depthStencilTextureView._texture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTextureView._texture->getFormat();
        attachmentDesc._samples = depthStencilTextureView._texture->getSamples();
        attachmentDesc._loadOp = depthOpState._loadOp;
        attachmentDesc._storeOp = depthOpState._storeOp;
        attachmentDesc._stencilLoadOp = stencilOpState._loadOp;
        attachmentDesc._stencilStoreOp = stencilOpState._storeOp;
        attachmentDesc._transition = tansitionState._state;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._imageViews.back() = depthStencilTextureView;
        m_attachmentsDesc._clearDepthValue = depthOpState._clearDepth;
        m_attachmentsDesc._clearStencilValue = stencilOpState._clearStencil;

        m_renderpassDesc._hasDepthStencilAttachment = true;

        return checkCompatibility(depthStencilTextureView, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._imageViews.back() = TextureView();

        m_renderpassDesc._hasDepthStencilAttachment = false;
    }

    return false;
}

bool RenderTargetState::checkCompatibility(const TextureView& texture, AttachmentDesc& desc)
{
    s32 layer = texture._subresource._baseLayer;
    if (layer != k_generalLayer && static_cast<u32>(layer) >= texture._texture->getLayersCount())
    {
        return false;
    }

    if (texture._texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer)) //backbuffer
    {
        if (desc._storeOp != RenderTargetStoreOp::StoreOp_Store) //backbuffer must be stored
        {
            return false;
        }
    }

    u32 countLayers = 0;
    for (u32 i = 0; i < std::numeric_limits<u16>::digits; ++i)
    {
        if ((m_attachmentsDesc._viewsMask >> i) & 0x1)
        {
            countLayers = i;
        }
    }

    if (countLayers > texture._texture->getLayersCount())
    {
        return false;
    }

    //TODO check format support
    return true;
}

void RenderTargetState::destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers)
{
    for (auto framebuffer : framebuffers)
    {
        m_device->destroyFramebuffer(framebuffer);
    }
}

void RenderTargetState::destroyRenderPasses(const std::vector<RenderPass*>& renderPasses)
{
    for (auto renderpass : renderPasses)
    {
        m_device->destroyRenderpass(renderpass);
    }
}

} //namespace renderer
} //namespace v3d
