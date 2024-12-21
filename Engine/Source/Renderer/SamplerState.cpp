#include "SamplerState.h"
#include "Utils/Logger.h"
#include "Device.h"

namespace v3d
{
namespace renderer
{

SamplerState::SamplerState(Device* device) noexcept
    : m_device(device)
    , m_tracker(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    LOG_DEBUG("SamplerState::SamplerState constructor %llx", this);

    memset(&m_samplerDesc, 0, sizeof(m_samplerDesc));
}

SamplerState::SamplerState(Device* device, SamplerFilter filter, SamplerAnisotropic aniso) noexcept
    : m_device(device)
    , m_tracker(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    LOG_DEBUG("SamplerState::SamplerState constructor %llx", this);

    memset(&m_samplerDesc, 0, sizeof(m_samplerDesc));
    m_samplerDesc._filter = filter;
    m_samplerDesc._anisotropic = aniso;
}

SamplerState::SamplerState(Device* device, const SamplerDesc& desc) noexcept
    : m_device(device)
    , m_samplerDesc(desc)
    , m_tracker(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    LOG_DEBUG("SamplerState::SamplerState constructor %llx", this);
}

SamplerState::~SamplerState()
{
    LOG_DEBUG("SamplerState::SamplerState destructor %llx", this);

    m_tracker.release();
}

void SamplerState::destroySamplers(const std::vector<Sampler*>& samplers)
{
    for (auto& samler : samplers)
    {
        m_device->destroySampler(samler);
    }
}

} //namespace renderer
} //namespace v3d