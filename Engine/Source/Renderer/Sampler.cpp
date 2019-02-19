#include "Sampler.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace renderer
{

Sampler::Sampler() noexcept
    : m_key(0)
{
}

Sampler::~Sampler()
{
}

SamplerManager::SamplerManager(Context * context) noexcept
    : m_context(context)
{
}

SamplerManager::~SamplerManager()
{
    SamplerManager::clear();
}

void SamplerManager::handleNotify(utils::Observable * ob)
{
    //TODO
}

Sampler * SamplerManager::acquireSampler(const Sampler::SamplerInfo& samplerInfo)
{
    SamplerDescription pDesc;
    pDesc._info = samplerInfo;

    Sampler* sampler = nullptr;
    auto found = m_samplerList.emplace(pDesc._hash, sampler);
    if (found.second)
    {
        sampler = m_context->createSampler();
        sampler->m_key = pDesc._hash;

        if (!sampler->create(samplerInfo))
        {
            sampler->destroy();
            m_samplerList.erase(pDesc._hash);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = sampler;
        sampler->registerNotify(this);

        return sampler;
    }

    return found.first->second;
}

bool SamplerManager::removeSampler(const Sampler::SamplerInfo& samplerInfo)
{
    //SamplerDescription pDesc;
    //pDesc._info = samplerInfo;

    //auto iter = m_samplerList.find(pDesc._hash);
    //if (iter == m_samplerList.cend())
    //{
    //    LOG_DEBUG("RenderPassManager renderpass not found");
    //    ASSERT(false, "renderpass");
    //    return false;
    //}

    //RenderPass* renderpass = iter->second;
    //if (renderpass->linked())
    //{
    //    LOG_WARNING("RenderPassManager::removeRenderPass renderPass still linked, but reqested to delete");
    //    ASSERT(false, "renderpass");
    //    //return false;
    //}

    //renderpass->notifyObservers();

    //renderpass->destroy();
    //delete renderpass;

    return false;
}

bool SamplerManager::removeSampler(Sampler * sampler)
{
    //TODO
    return false;
}



void SamplerManager::clear()
{
    //TODO
}

} //namespace renderer
} //namespace v3d
