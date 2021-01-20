#include "Sampler.h"
#include "Renderer/Context.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

Sampler::Sampler(const SamplerDescription& desc) noexcept
    : m_desc(desc)
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

Sampler* SamplerManager::acquireSampler(const SamplerDescription& samplerDesc)
{
    Sampler* sampler = nullptr;
    auto found = m_samplerList.emplace(samplerDesc, sampler);
    if (found.second)
    {
        sampler = m_context->createSampler(samplerDesc);
        if (!sampler->create())
        {
            sampler->destroy();
            m_samplerList.erase(found.first);

            ASSERT(false, "can't create sampler");
            return nullptr;
        }
        found.first->second = sampler;
        sampler->registerNotify(this);

        return sampler;
    }

    return found.first->second;
}

bool SamplerManager::removeSampler(Sampler* sampler)
{
    auto iter = m_samplerList.find(sampler->m_desc);
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

void SamplerManager::handleNotify(const utils::Observable* ob)
{
    LOG_DEBUG("SamplerManager::handleNotify sampler %llx has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
