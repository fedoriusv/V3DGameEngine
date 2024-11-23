#include "RenderTargetState.h"
#include "Device.h"
#include "DeviceCaps.h"
#include "Texture.h"
#include "Utils/Logger.h"


namespace v3d
{
namespace renderer
{

RenderTargetState::RenderTargetState(Device* device, const math::Dimension2D& size, u32 countAttacments, u32 viewsMask, const std::string& name) noexcept
    : m_device(device)
    , m_name(name)

    , m_trackerFramebuffer(this, std::bind(&RenderTargetState::destroyFramebuffers, this, std::placeholders::_1))
    , m_trackerRenderpass(this, std::bind(&RenderTargetState::destroyRenderPasses, this, std::placeholders::_1))
{
    LOG_DEBUG("RenderTargetState::RenderTargetState constructor %llx", this);

    ASSERT(countAttacments < m_device->getDeviceCaps()._maxColorAttachments, "index >= maxColorattachments");
    m_renderpassDesc._countColorAttachments = countAttacments;
    m_attachmentsDesc._renderArea = size;
    m_attachmentsDesc._viewsMask = viewsMask;

    m_renderTargets.fill(nullptr);
}

RenderTargetState::~RenderTargetState()
{
    LOG_DEBUG("RenderTargetState::RenderTargetState destructor %llx", this);
    m_trackerFramebuffer.release();
    m_trackerRenderpass.release();
}

bool RenderTargetState::setColorTexture_Impl(u32 index, Texture* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const renderer::Color& clearColor)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    if (colorTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSamples();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(colorTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
        m_attachmentsDesc._images[index] = colorTexture->m_texture;
        m_attachmentsDesc._layers[index] = k_generalLayer;
        m_attachmentsDesc._clearColorValues[index] = clearColor;
        m_renderTargets[index] = colorTexture;

        return checkCompatibility(colorTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setColorTexture_Impl(u32 index, Texture* colorTexture, u32 layer, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const renderer::Color& clearColor)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    ASSERT(colorTexture->getTarget() == TextureTarget::TextureCubeMap && layer < 6U, "index >= max 6 sides for cubemap");
    if (colorTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSamples();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(colorTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(layer);
        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
        m_attachmentsDesc._images[index] = colorTexture->m_texture;
        m_attachmentsDesc._layers[index] = k_generalLayer;
        m_attachmentsDesc._clearColorValues[index] = clearColor;
        m_renderTargets[index] = colorTexture;

        return checkCompatibility(colorTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setColorTexture_Impl(u32 index, Texture* colorTexture, const ColorOpState& colorOpState, const TransitionState& tansitionState)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    if (colorTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSamples();
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(colorTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._images[index] = colorTexture->m_texture;
        m_attachmentsDesc._layers[index] = k_generalLayer;
        m_attachmentsDesc._clearColorValues[index] = Color(0);
        m_renderTargets[index] = colorTexture;

        return checkCompatibility(colorTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setColorTexture_Impl(u32 index, Texture* colorTexture, s32 layer, const ColorOpState& colorOpState, const TransitionState& tansitionState)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    ASSERT(colorTexture->getTarget() == TextureTarget::TextureCubeMap && layer < 6U, "index >= max 6 sides for cubemap");
    if (colorTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSamples();
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(colorTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(layer);
        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._images[index] = colorTexture->m_texture;
        m_attachmentsDesc._layers[index] = 0;
        m_attachmentsDesc._clearColorValues[index] = Color(0);
        m_renderTargets[index] = colorTexture;

        return checkCompatibility(colorTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setSwapchainTexture_Impl(u32 index, SwapchainTexture* swapchainTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const renderer::Color& clearColor)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    if (swapchainTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = swapchainTexture->getFormat();
        attachmentDesc._samples = swapchainTexture->getSamples();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = true;
        attachmentDesc._autoResolve = false;
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
        m_attachmentsDesc._images[index] = TextureHandle(swapchainTexture->m_swapchain);
        m_attachmentsDesc._layers[index] = k_generalLayer;
        m_attachmentsDesc._clearColorValues[index] = clearColor;
        m_renderTargets[index] = swapchainTexture;

        return checkCompatibility(swapchainTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setSwapchainTexture_Impl(u32 index, SwapchainTexture* swapchainTexture, const ColorOpState& colorOpState, const TransitionState& tansitionState)
{
    ASSERT(index < m_renderpassDesc._countColorAttachments, "index >= count of attachments");
    if (swapchainTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc[index];
        attachmentDesc._format = swapchainTexture->getFormat();
        attachmentDesc._samples = swapchainTexture->getSamples();
        attachmentDesc._loadOp = colorOpState._loadOp;
        attachmentDesc._storeOp = colorOpState._storeOp;
        attachmentDesc._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        attachmentDesc._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        attachmentDesc._backbuffer = true;
        attachmentDesc._autoResolve = false;
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._images[index] = TextureHandle(swapchainTexture->m_swapchain);
        m_attachmentsDesc._layers[index] = k_generalLayer;
        m_attachmentsDesc._clearColorValues[index] = Color(0);
        m_renderTargets[index] = swapchainTexture;

        return checkCompatibility(swapchainTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc[index] = AttachmentDesc();
        m_attachmentsDesc._images[index] = TextureHandle();
        m_attachmentsDesc._layers[index] = 0;
        m_renderTargets[index] = nullptr;
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture_Impl(Texture* depthStencilTexture, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
{
    if (depthStencilTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSamples();
        attachmentDesc._loadOp = depthLoadOp;
        attachmentDesc._storeOp = depthStoreOp;
        attachmentDesc._stencilLoadOp = stencilLoadOp;
        attachmentDesc._stencilStoreOp = stencilStoreOp;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(depthStencilTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
        m_attachmentsDesc._images.back() = depthStencilTexture->m_texture;
        m_attachmentsDesc._layers.back() = k_generalLayer;
        m_attachmentsDesc._clearDepthValue = clearDepth;
        m_attachmentsDesc._clearStencilValue = clearStencil;
        m_renderTargets.back() = depthStencilTexture;

        m_renderpassDesc._hasDepthStencilAttahment = true;

        return checkCompatibility(depthStencilTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._images.back() = TextureHandle();
        m_attachmentsDesc._layers.back() = 0;
        m_renderTargets.back() = nullptr;

        m_renderpassDesc._hasDepthStencilAttahment = false;
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture_Impl(Texture* depthStencilTexture, u32 layer, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
{
    ASSERT(depthStencilTexture->getTarget() == TextureTarget::TextureCubeMap && layer < 6U, "index >= max 6 sides for cubemap");
    if (depthStencilTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSamples();
        attachmentDesc._loadOp = depthLoadOp;
        attachmentDesc._storeOp = depthStoreOp;
        attachmentDesc._stencilLoadOp = stencilLoadOp;
        attachmentDesc._stencilStoreOp = stencilStoreOp;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(depthStencilTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(layer);
        attachmentDesc._initTransition = TransitionOp::TransitionOp_Undefined;
        attachmentDesc._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
        m_attachmentsDesc._images.back() = depthStencilTexture->m_texture;
        m_attachmentsDesc._layers.back() = layer;
        m_attachmentsDesc._clearDepthValue = clearDepth;
        m_attachmentsDesc._clearStencilValue = clearStencil;
        m_renderTargets.back() = depthStencilTexture;

        m_renderpassDesc._hasDepthStencilAttahment = true;

        return checkCompatibility(depthStencilTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._images.back() = TextureHandle();
        m_attachmentsDesc._layers.back() = 0;
        m_renderTargets.back() = nullptr;

        m_renderpassDesc._hasDepthStencilAttahment = false;
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture_Impl(Texture* depthStencilTexture, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState)
{
    if (depthStencilTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSamples();
        attachmentDesc._loadOp = depthOpState._loadOp;
        attachmentDesc._storeOp = depthOpState._storeOp;
        attachmentDesc._stencilLoadOp = stencilOpState._loadOp;
        attachmentDesc._stencilStoreOp = stencilOpState._storeOp;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(depthStencilTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(k_generalLayer);
        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._images.back() = depthStencilTexture->m_texture;
        m_attachmentsDesc._layers.back() = k_generalLayer;
        m_attachmentsDesc._clearDepthValue = 0.f;
        m_attachmentsDesc._clearStencilValue = 0;
        m_renderTargets.back() = depthStencilTexture;

        m_renderpassDesc._hasDepthStencilAttahment = true;

        return checkCompatibility(depthStencilTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._images.back() = TextureHandle();
        m_attachmentsDesc._layers.back() = 0;
        m_renderTargets.back() = nullptr;

        m_renderpassDesc._hasDepthStencilAttahment = false;
    }

    return false;
}

bool RenderTargetState::setDepthStencilTexture_Impl(Texture* depthStencilTexture, s32 layer, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState)
{
    if (depthStencilTexture) [[likely]]
    {
        AttachmentDesc& attachmentDesc = m_renderpassDesc._attachmentsDesc.back();
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSamples();
        attachmentDesc._loadOp = depthOpState._loadOp;
        attachmentDesc._storeOp = depthOpState._storeOp;
        attachmentDesc._stencilLoadOp = stencilOpState._loadOp;
        attachmentDesc._stencilStoreOp = stencilOpState._storeOp;
        attachmentDesc._backbuffer = false;
        attachmentDesc._autoResolve = OBJECT_FROM_HANDLE(depthStencilTexture->m_texture, RenderTexture)->hasUsageFlag(TextureUsage::TextureUsage_Resolve);
        attachmentDesc._layer = AttachmentDesc::compressLayer(layer);
        attachmentDesc._initTransition = tansitionState._initialState;
        attachmentDesc._finalTransition = tansitionState._finalState;
        m_attachmentsDesc._images.back() = depthStencilTexture->m_texture;
        m_attachmentsDesc._layers.back() = layer;
        m_attachmentsDesc._clearDepthValue = 0.f;
        m_attachmentsDesc._clearStencilValue = 0;
        m_renderTargets.back() = depthStencilTexture;

        m_renderpassDesc._hasDepthStencilAttahment = true;

        return checkCompatibility(depthStencilTexture, attachmentDesc);
    }
    else
    {
        m_renderpassDesc._attachmentsDesc.back() = AttachmentDesc();
        m_attachmentsDesc._images.back() = TextureHandle();
        m_attachmentsDesc._layers.back() = 0;
        m_renderTargets.back() = nullptr;

        m_renderpassDesc._hasDepthStencilAttahment = false;
    }

    return false;
}

bool RenderTargetState::checkCompatibility(Texture* texture, AttachmentDesc& desc)
{
    s32 layer = AttachmentDesc::uncompressLayer(desc._layer);
    if (layer != k_generalLayer && static_cast<u32>(layer) >= texture->getLayersCount())
    {
        return false;
    }

    if (desc._backbuffer) //backbuffer
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

    if (countLayers > texture->getLayersCount())
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
