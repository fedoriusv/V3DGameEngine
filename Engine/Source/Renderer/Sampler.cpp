#include "Sampler.h"
#include "Renderer/Context.h"

#include "Utils/Logger.h"

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

Sampler * SamplerManager::acquireSampler(const SamplerDescription& samplerInfo)
{
    Sampler::SamplerInfo::SamplerDesc info;
    info._desc = samplerInfo;

    Sampler* sampler = nullptr;
    auto found = m_samplerList.emplace(info._hash, sampler);
    if (found.second)
    {
        sampler = m_context->createSampler();
        sampler->m_key = info._hash;

        if (!sampler->create(info._desc))
        {
            sampler->destroy();
            m_samplerList.erase(info._hash);

            ASSERT(false, "can't create sampler");
            return nullptr;
        }
        found.first->second = sampler;
        sampler->registerNotify(this);

        return sampler;
    }

    return found.first->second;
}

bool SamplerManager::removeSampler(Sampler * sampler)
{
    auto iter = m_samplerList.find(sampler->m_key);
    if (iter == m_samplerList.cend())
    {
        LOG_DEBUG("SamplerManager sampler not found");
        ASSERT(false, "sampler");
        return false;
    }

    Sampler* samplerIter = iter->second;
    ASSERT(samplerIter == sampler, "Different pointers");
    if (samplerIter->linked())
    {
        LOG_WARNING("PipelineManager::removePipeline sampler still linked, but reqested to delete");
        ASSERT(false, "sampler");
        //return false;
    }
    m_samplerList.erase(iter);

    samplerIter->notifyObservers();

    samplerIter->destroy();
    delete  samplerIter;

    return true;
}

void SamplerManager::clear()
{
    for (auto& iter : m_samplerList)
    {
        Sampler* sampler = iter.second;
        if (sampler->linked())
        {
            LOG_WARNING("SamplerManager::clear sampler still linked, but reqested to delete");
            ASSERT(false, "sampler");
        }
        sampler->notifyObservers();

        sampler->destroy();
        delete sampler;
    }
    m_samplerList.clear();
}

void SamplerManager::handleNotify(utils::Observable * ob)
{
    LOG_DEBUG("SamplerManager sampler %x has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
