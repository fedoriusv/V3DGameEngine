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
    for (u32 index = 0; index < renderpassDesc._countColorAttachments; ++index)
    {
        compatibilityRenderpassDesc._attachmentsDesc[index]._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc[index]._initTransition = TransitionOp::TransitionOp_Undefined;
        compatibilityRenderpassDesc._attachmentsDesc[index]._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
#ifndef PLATFORM_ANDROID //used for pretransform
        compatibilityRenderpassDesc._attachmentsDesc[index]._backbuffer = false;
#endif
        compatibilityRenderpassDesc._attachmentsDesc[index]._layer = 0;
    }

    if (compatibilityRenderpassDesc._hasDepthStencilAttahment)
    {
        compatibilityRenderpassDesc._attachmentsDesc.back()._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._attachmentsDesc.back()._initTransition = TransitionOp::TransitionOp_Undefined;
        compatibilityRenderpassDesc._attachmentsDesc.back()._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
#ifndef PLATFORM_ANDROID //used for pretransform
        compatibilityRenderpassDesc._attachmentsDesc.back()._backbuffer = false;
#endif
        compatibilityRenderpassDesc._attachmentsDesc.back()._layer = 0;
    }

    return compatibilityRenderpassDesc;
}

} //namespace renderer
} //namespace v3d
