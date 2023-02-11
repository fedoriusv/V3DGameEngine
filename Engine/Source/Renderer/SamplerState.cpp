#include "SamplerState.h"
#include "Core/Context.h"
#include "Core/Sampler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

SamplerState::SamplerState(renderer::CommandList& cmdList) noexcept
    : m_cmdList(cmdList)
    , m_trackerSampler(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    m_samplerDesc.dirty();
}

SamplerState::SamplerState(renderer::CommandList& cmdList, SamplerFilter filter, SamplerAnisotropic aniso) noexcept
    : m_cmdList(cmdList)
    , m_trackerSampler(this, std::bind(&SamplerState::destroySamplers, this, std::placeholders::_1))
{
    m_samplerDesc._desc._filter = filter;
    m_samplerDesc._desc._anisotropic = aniso;
    m_samplerDesc.dirty();
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
        /*CommandRemoveSamplers*/
        class CommandRemoveSamplers final : public Command
        {
        public:

            explicit CommandRemoveSamplers(const std::vector<Sampler*>& sampler) noexcept
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

        m_cmdList.pushCommand(new CommandRemoveSamplers(samplers));
    }
}

SamplerState::~SamplerState()
{
    LOG_DEBUG("SamplerState::SamplerState destructor %llx", this);
    m_trackerSampler.release();
}

void SamplerState::setFiltering(SamplerFilter filter)
{
    m_samplerDesc._desc._filter = filter;
    m_samplerDesc.dirty();
}

void SamplerState::setWrap(SamplerWrap uvw)
{
    m_samplerDesc._desc._wrapU = uvw;
    m_samplerDesc._desc._wrapV = uvw;
    m_samplerDesc._desc._wrapW = uvw;
    m_samplerDesc.dirty();
}

void SamplerState::setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w)
{
    m_samplerDesc._desc._wrapU = u;
    m_samplerDesc._desc._wrapV = v;
    m_samplerDesc._desc._wrapW = w;
    m_samplerDesc.dirty();
}

void SamplerState::setAnisotropic(SamplerAnisotropic level)
{
    m_samplerDesc._desc._anisotropic = level;
    m_samplerDesc.dirty();
}

void SamplerState::setLodBias(f32 value)
{
    m_samplerDesc._desc._lodBias = value;
    m_samplerDesc.dirty();
}

void SamplerState::setCompareOp(CompareOperation op)
{
    m_samplerDesc._desc._compareOp = op;
    m_samplerDesc.dirty();
}

void SamplerState::setEnableCompareOp(bool enable)
{
    m_samplerDesc._desc._enableCompOp = enable;
    m_samplerDesc.dirty();
}

void SamplerState::setBorderColor(const core::Vector4D& color)
{
    m_samplerDesc._desc._borderColor = color;
    m_samplerDesc.dirty();
}

} //namespace renderer
} //namespace v3d
