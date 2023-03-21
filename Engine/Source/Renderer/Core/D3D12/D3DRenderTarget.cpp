#include "D3DRenderTarget.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DImage.h"
#include "D3DDescriptorHeap.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3D12_RESOURCE_STATES D3DRenderState::convertTransitionToD3D(TransitionOp transition)
{
    switch (transition)
    {
    case TransitionOp::TransitionOp_ShaderRead:
        return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    case TransitionOp::TransitionOp_ColorAttachment:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;

    case TransitionOp::TransitionOp_DepthStencilAttachment:
        return D3D12_RESOURCE_STATE_DEPTH_WRITE;

    case TransitionOp::TransitionOp_Present:
        return D3D12_RESOURCE_STATE_PRESENT;

    case TransitionOp::TransitionOp_GeneralGraphic:
        return D3D12_RESOURCE_STATE_COMMON;

    case TransitionOp::TransitionOp_GeneralCompute:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    default:
        ASSERT(false, "unknown");
    }

    return D3D12_RESOURCE_STATE_COMMON;
}

D3DRenderState::D3DRenderState(const RenderPassDescription& desc) noexcept
    : RenderPass(desc)
{
}

bool D3DRenderState::create()
{
    return true;
}

void D3DRenderState::destroy()
{
}

D3DRenderTarget::D3DRenderTarget(ID3D12Device* device, D3DDescriptorHeapManager* manager, const std::vector<Image*>& images) noexcept
    : m_device(device)
    , m_heapManager(manager)
    , m_renderState(nullptr)
    , m_images(images)

    , m_colorDescriptorHeap(nullptr)
    , m_depthStencilDescriptorHeap(nullptr)

    , m_depthStencilRenderTarget()
{
    LOG_DEBUG("D3DRenderTarget::D3DRenderTarget constructor %llx", this);
}

D3DRenderTarget::~D3DRenderTarget()
{
    LOG_DEBUG("D3DRenderTarget::~D3DRenderTarget descructor %llx", this);
}

bool D3DRenderTarget::create(const RenderPass* pass)
{
    m_renderState = static_cast<const D3DRenderState*>(pass);
    ASSERT(m_renderState, "nullptr");

    u32 countColor = m_renderState->getDescription()._countColorAttachments;
    m_colorRenderTargets.resize(countColor);

    m_colorDescriptorHeap = m_heapManager->allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, countColor, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    ASSERT(m_colorDescriptorHeap, "nullptr");

    for (auto image : m_images)
    {
        u32 index = 0;
        D3DImage* dxImage = static_cast<D3DImage*>(image);
        if (D3DImage::isColorFormat(dxImage->getFormat()))
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE colorHandle(m_colorDescriptorHeap->getCPUHandle(), index, m_colorDescriptorHeap->getIncrement());
            s32 layer = AttachmentDescription::uncompressLayer(m_renderState->getDescription()._attachments[index]._layer);

            const D3D12_RENDER_TARGET_VIEW_DESC& targetView = dxImage->getView<D3D12_RENDER_TARGET_VIEW_DESC>(D3DImage::makeD3DImageSubresource(dxImage, layer, 0));
            m_device->CreateRenderTargetView(dxImage->getResource(), &targetView, colorHandle);

            m_colorRenderTargets[index] = colorHandle;
        }
        else
        {
            m_depthStencilDescriptorHeap = m_heapManager->allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
            ASSERT(m_depthStencilDescriptorHeap, "nullptr");
            CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilHandle(m_depthStencilDescriptorHeap->getCPUHandle());
            s32 layer = AttachmentDescription::uncompressLayer(m_renderState->getDescription()._attachments.back()._layer);

            const D3D12_DEPTH_STENCIL_VIEW_DESC& targetView = dxImage->getView<D3D12_DEPTH_STENCIL_VIEW_DESC>(D3DImage::makeD3DImageSubresource(dxImage, layer, 0));
            m_device->CreateDepthStencilView(dxImage->getResource(), &targetView, depthStencilHandle);

            m_depthStencilRenderTarget = depthStencilHandle;
        }

        ++index;
    }

    return true;
}

void D3DRenderTarget::destroy()
{
    if (m_colorDescriptorHeap)
    {
        m_heapManager->deallocDescriptorHeap(m_colorDescriptorHeap);
    }

    if (m_depthStencilDescriptorHeap)
    {
        m_heapManager->deallocDescriptorHeap(m_depthStencilDescriptorHeap);
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER