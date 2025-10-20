#include "Pipeline.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

RenderPipeline::RenderPipeline(PipelineType type) noexcept
    : m_pipelineType(type)
{
}

RenderPipeline::~RenderPipeline()
{
}

const RenderPassDesc RenderPipeline::createCompatibilityRenderPassDescription(const RenderPassDesc& renderpassDesc)
{
    RenderPassDesc compatibilityRenderpassDesc(renderpassDesc);
    for (u32 index = 0; index < renderpassDesc._countColorAttachment; ++index)
    {
        compatibilityRenderpassDesc._attachmentsDesc[index]._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._transition = TransitionOp::TransitionOp_ColorAttachment;
        compatibilityRenderpassDesc._attachmentsDesc[index]._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
    }

    if (compatibilityRenderpassDesc._hasDepthStencilAttachment)
    {
        compatibilityRenderpassDesc._attachmentsDesc.back()._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._transition = TransitionOp::TransitionOp_DepthStencilAttachment;
        compatibilityRenderpassDesc._attachmentsDesc.back()._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
    }

    return compatibilityRenderpassDesc;
}

} //namespace renderer
} //namespace v3d
