#include "Pipeline.h"
#include "Context.h"
#include "Utils/Logger.h"
#include "Renderer/Shader.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

const RenderPassDescription Pipeline::createCompatibilityRenderPassDescription(const RenderPassDescription& renderpassDesc)
{
    RenderPassDescription compatibilityRenderpassDesc(renderpassDesc);
    for (u32 index = 0; index < renderpassDesc._desc._countColorAttachments; ++index)
    {
        compatibilityRenderpassDesc._desc._attachments[index]._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments[index]._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments[index]._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments[index]._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments[index]._initTransition = TransitionOp::TransitionOp_Undefined;
        compatibilityRenderpassDesc._desc._attachments[index]._finalTransition = TransitionOp::TransitionOp_ColorAttachment;
#ifndef PLATFORM_ANDROID //used for pretransform
        compatibilityRenderpassDesc._desc._attachments[index]._backbuffer = false;
#endif
        compatibilityRenderpassDesc._desc._attachments[index]._layer = 0;
    }

    if (compatibilityRenderpassDesc._desc._hasDepthStencilAttahment)
    {
        compatibilityRenderpassDesc._desc._attachments.back()._loadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments.back()._storeOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments.back()._stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments.back()._stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare;
        compatibilityRenderpassDesc._desc._attachments.back()._initTransition = TransitionOp::TransitionOp_Undefined;
        compatibilityRenderpassDesc._desc._attachments.back()._finalTransition = TransitionOp::TransitionOp_DepthStencilAttachment;
#ifndef PLATFORM_ANDROID //used for pretransform
        compatibilityRenderpassDesc._desc._attachments.back()._backbuffer = false;
#endif
        compatibilityRenderpassDesc._desc._attachments.back()._layer = 0;
    }

    return compatibilityRenderpassDesc;
}

Pipeline::Pipeline(PipelineType type) noexcept
    : m_pipelineType(type)
{
}

Pipeline::~Pipeline() 
{
}

Pipeline::PipelineType Pipeline::getType() const
{
    return m_pipelineType;
}

bool Pipeline::createShader(const Shader* shader)
{
    if (!shader)
    {
        return false;
    }

    return compileShader(shader->getShaderType(), shader->m_source, shader->m_size);
}

bool Pipeline::createProgram(const ShaderProgramDescription& desc)
{
    std::vector<std::tuple<ShaderType, const void*, u32>> shadersData;
    for (u32 type = toEnumType(ShaderType::Vertex); type < (u32)toEnumType(ShaderType::Count); ++type)
    {
        const Shader* shader = desc._shaders[type];
        if (!shader)
        {
            continue;
        }

        shadersData.push_back({ shader->getShaderType(), shader->m_source, shader->m_size });
    }

    return compileShaders(shadersData);
}

bool Pipeline::compileShader(ShaderType type, const void* source, u32 size)
{
    return false;
}

bool Pipeline::compileShaders(std::vector<std::tuple<ShaderType, const void*, u32>>& shaders)
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::PipelineDescription::PipelineDescription() noexcept
    : _pipelineType(Pipeline::PipelineType::PipelineType_Graphic)
    , _hash(0)
{
}

Pipeline::PipelineDescription::PipelineDescription(const PipelineGraphicInfo& pipelineInfo) noexcept
    : _pipelineDesc(pipelineInfo._pipelineDesc)
    , _renderpassDesc(Pipeline::createCompatibilityRenderPassDescription(pipelineInfo._renderpassDesc))
    , _programDesc(pipelineInfo._programDesc)
    , _pipelineType(Pipeline::PipelineType::PipelineType_Graphic)
    , _hash(0)
{
    //u32 hash = crc32c::Crc32c((u8*)&_pipelineDesc, sizeof(GraphicsPipelineStateDescription));
    //hash = crc32c::Extend(hash, (u8*)&_renderpassDesc, sizeof(RenderPass::RenderPassInfo));

    //Use only shader hash, due Crc32 has a lot of collision for pipeline
    _hash = _programDesc._hash;
    //_hash = hash | _hash << 32;
}

Pipeline::PipelineDescription::PipelineDescription(const PipelineComputeInfo& pipelineInfo) noexcept
    : _programDesc(pipelineInfo._programDesc)
    , _pipelineType(Pipeline::PipelineType::PipelineType_Compute)
    , _hash(0)
{
    _hash = pipelineInfo._programDesc._hash;
}

u64 Pipeline::PipelineDescription::Hash::operator()(const PipelineDescription& desc) const
{
    ASSERT(desc._hash, "must be not 0");
    return desc._hash;
}

bool Pipeline::PipelineDescription::Compare::operator()(const PipelineDescription& op1, const PipelineDescription& op2) const
{
    ASSERT(op1._hash != 0 && op2._hash != 0, "must be not 0");
    if (op1._hash != op2._hash)
    {
        return false;
    }

    ASSERT(op1._pipelineType == op2._pipelineType, "diff types");
    if (op1._pipelineType == Pipeline::PipelineType::PipelineType_Graphic)
    {
        if (op1._pipelineDesc != op2._pipelineDesc)
        {
            return false;
        }

        if (op1._renderpassDesc != op2._renderpassDesc)
        {
            return false;
        }

        if (op1._programDesc._shaders != op2._programDesc._shaders)
        {
            return false;
        }

        return true;
    }
    else //Pipeline::PipelineType::PipelineType_Compute
    {
        return op1._programDesc._shaders == op2._programDesc._shaders;
    }
}

PipelineManager::PipelineManager(Context* context) noexcept
    : m_context(context)
{
}

PipelineManager::~PipelineManager()
{
    PipelineManager::clear();
}

Pipeline* PipelineManager::acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo& pipelineInfo)
{
    Pipeline::PipelineDescription desc(pipelineInfo);

    Pipeline* pipeline = nullptr;
    auto found = m_pipelineGraphicList.emplace(desc, pipeline);
    if (found.second)
    {
        pipeline = m_context->createPipeline(Pipeline::PipelineType::PipelineType_Graphic, pipelineInfo._name);
        pipeline->m_desc = desc;

        if (!pipeline->create(&pipelineInfo))
        {
            pipeline->destroy();
            m_pipelineGraphicList.erase(desc);

            ASSERT(false, "can't create pipeline");
            return nullptr;
        }

        found.first->second = pipeline;
        pipeline->registerNotify(this);

        return pipeline;
    }

    return found.first->second;
}

Pipeline* PipelineManager::acquireComputePipeline(const Pipeline::PipelineComputeInfo& pipelineInfo)
{
    Pipeline::PipelineDescription desc(pipelineInfo);

    Pipeline* pipeline = nullptr;
    auto found = m_pipelineComputeList.emplace(desc, pipeline);
    if (found.second)
    {
        pipeline = m_context->createPipeline(Pipeline::PipelineType::PipelineType_Compute, pipelineInfo._name);
        pipeline->m_desc = desc;

        if (!pipeline->create(&pipelineInfo))
        {
            pipeline->destroy();
            m_pipelineComputeList.erase(desc);

            ASSERT(false, "can't create pipeline");
            return nullptr;
        }
        found.first->second = pipeline;
        pipeline->registerNotify(this);

        return pipeline;
    }

    return found.first->second;
}

bool PipelineManager::removePipeline(Pipeline* pipeLine)
{
    ASSERT(pipeLine->getType() == Pipeline::PipelineType::PipelineType_Graphic || pipeLine->getType() == Pipeline::PipelineType::PipelineType_Compute, "wrong type");
    auto& pipelineList = (pipeLine->getType() == Pipeline::PipelineType::PipelineType_Compute) ? m_pipelineComputeList : m_pipelineGraphicList;

    auto iter = pipelineList.find(pipeLine->m_desc);
    if (iter == pipelineList.cend())
    {
        LOG_DEBUG("PipelineManager pipeline not found");
        ASSERT(false, "pipeline");
        return false;
    }

    Pipeline* pipeline = iter->second;
    ASSERT(pipeline == pipeLine, "Different pointers");
    if (pipeline->linked())
    {
        LOG_WARNING("PipelineManager::removePipeline pipleline still linked, but reqested to delete");
        ASSERT(false, "pipeline");
        //return false;
    }
    pipelineList.erase(iter);

    pipeline->notifyObservers();

    pipeline->destroy();
    delete  pipeline;

    return true;
}

void PipelineManager::clear()
{
    for (auto& iter : m_pipelineGraphicList)
    {
        Pipeline* pipeline = iter.second;
        if (pipeline->linked())
        {
            LOG_WARNING("PipelineManager::clear pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
        }
        pipeline->notifyObservers();

        pipeline->destroy();
        delete pipeline;
    }
    m_pipelineGraphicList.clear();

    for (auto& iter : m_pipelineComputeList)
    {
        Pipeline* pipeline = iter.second;
        if (pipeline->linked())
        {
            LOG_WARNING("PipelineManager::clear pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
        }
        pipeline->notifyObservers();

        pipeline->destroy();
        delete pipeline;
    }
    m_pipelineComputeList.clear();
}

void PipelineManager::handleNotify(const utils::Observable* object, void* msg)
{
    LOG_DEBUG("PipelineManager pipeline %x has been deleted", object);
}

} //namespace renderer
} //namespace v3d
