#include "TextureSampler.h"

namespace v3d
{
namespace renderer
{

SamplerState::SamplerState(renderer::CommandList & cmdList) noexcept
    : m_cmdList(cmdList)

    , m_filter(renderer::SamplerFilter_Nearest)
    , m_anisotropicLevel(renderer::SamplerAnisotropic::SamplerAnisotropic_None)
    , m_wrap(renderer::SamplerWrap::TextureWrap_Repeat)
{
}

void SamplerState::handleNotify(utils::Observable * ob)
{
   //TODO:
}

renderer::SamplerFilter SamplerState::getMinFilter() const
{
    return renderer::SamplerFilter_Nearest;
}

renderer::SamplerFilter SamplerState::getMagFilter() const
{
    return renderer::SamplerFilter_Nearest;
}

renderer::SamplerWrap SamplerState::getWrap() const
{
    return m_wrap;
}

renderer::SamplerAnisotropic SamplerState::getAnisotropic() const
{
    return m_anisotropicLevel;
}

} //namespace renderer
} //namespace v3d
