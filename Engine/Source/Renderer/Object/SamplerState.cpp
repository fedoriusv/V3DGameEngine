#include "SamplerState.h"
#include "Renderer/Context.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CommandRemoveSamplers*/
class CommandRemoveSamplers final : public Command
{
public:
    CommandRemoveSamplers(const std::vector<Sampler*>& sampler) noexcept
        : m_samplers(sampler)
    {
        LOG_DEBUG("CommandRemoveSamplers constructor");
    };
    CommandRemoveSamplers() = delete;
    CommandRemoveSamplers(CommandRemoveSamplers&) = delete;

    ~CommandRemoveSamplers()
    {
        LOG_DEBUG("CommandRemoveSamplers destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandRemoveSamplers execute");
        for (auto& sampler : m_samplers)
        {
            cmdList.getContext()->removeSampler(sampler);
        }
    }

private:
    const std::vector<Sampler*> m_samplers;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

SamplerState::SamplerState(renderer::CommandList & cmdList) noexcept
    : m_cmdList(cmdList)
    , m_trackerSampler(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
}

SamplerState::SamplerState(renderer::CommandList & cmdList, SamplerFilter filter, SamplerAnisotropic aniso) noexcept
    : m_cmdList(cmdList)
    , m_trackerSampler(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    m_samplerDesc._minFilter = filter;
    m_samplerDesc._magFilter = filter;
    m_samplerDesc._anisotropic = aniso;
}

SamplerState::SamplerState(renderer::CommandList & cmdList, SamplerFilter min, SamplerFilter mag, SamplerAnisotropic aniso) noexcept
    : m_cmdList(cmdList)
    , m_trackerSampler(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    m_samplerDesc._minFilter = min;
    m_samplerDesc._magFilter = mag;
    m_samplerDesc._anisotropic = aniso;
}

void SamplerState::handleNotify(utils::Observable * ob)
{
   //TODO:
}

void SamplerState::destroySamplers(const std::vector<Sampler*>& samplers)
{
    if (m_cmdList.isImmediate())
    {
        for (auto& sampler : samplers)
        {
            m_cmdList.getContext()->removeSampler(sampler);
        }
    }
    else
    {
        m_cmdList.pushCommand(new CommandRemoveSamplers(samplers));
    }
}

SamplerState::~SamplerState()
{
    m_trackerSampler.release();
}

renderer::SamplerFilter SamplerState::getMinFilter() const
{
    return m_samplerDesc._minFilter;
}

renderer::SamplerFilter SamplerState::getMagFilter() const
{
    return m_samplerDesc._magFilter;
}

SamplerWrap SamplerState::getWrapU() const
{
    return m_samplerDesc._wrapU;
}

SamplerWrap SamplerState::getWrapV() const
{
    return m_samplerDesc._wrapV;
}

SamplerWrap SamplerState::getWrapW() const
{
    return m_samplerDesc._wrapW;
}

SamplerAnisotropic SamplerState::getAnisotropic() const
{
    return m_samplerDesc._anisotropic;
}

CompareOperation SamplerState::getCompareOp() const
{
    return m_samplerDesc._compareOp;
}

bool SamplerState::isEnableCompareOp() const
{
    return m_samplerDesc._enableCompOp;
}

void SamplerState::setMinFilter(SamplerFilter filter)
{
    m_samplerDesc._minFilter = filter;
}

void SamplerState::setMagFilter(SamplerFilter filter)
{
    m_samplerDesc._magFilter = filter;
}

void SamplerState::setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w)
{
    m_samplerDesc._wrapU = u;
    m_samplerDesc._wrapV = v;
    m_samplerDesc._wrapW = w;
}

void SamplerState::setAnisotropic(SamplerAnisotropic level)
{
    m_samplerDesc._anisotropic = level;
}

void SamplerState::setLodBias(f32 value)
{
    m_samplerDesc._lodBias = value;
}

void SamplerState::setCompareOp(CompareOperation op)
{
    m_samplerDesc._compareOp = op;
}

void SamplerState::setEnableCompareOp(bool enable)
{
    m_samplerDesc._enableCompOp = enable;
}

} //namespace renderer
} //namespace v3d
