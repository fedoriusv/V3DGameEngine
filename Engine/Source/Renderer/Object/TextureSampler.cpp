#include "TextureSampler.h"

namespace v3d
{
namespace renderer
{

TextureSampler::TextureSampler(renderer::CommandList & cmdList) noexcept
    : m_cmdList(cmdList)

    , m_filter(renderer::TextureFilter_Nearest)
    , m_anisotropicLevel(renderer::TextureAnisotropic::TextureAnisotropic_None)
    , m_wrap(renderer::TextureWrap::TextureWrap_Repeat)
{
}

renderer::TextureFilter TextureSampler::getMinFilter() const
{
    return renderer::TextureFilter_Nearest;
}

renderer::TextureFilter TextureSampler::getMagFilter() const
{
    return renderer::TextureFilter_Nearest;
}

renderer::TextureWrap TextureSampler::getWrap() const
{
    return m_wrap;
}

renderer::TextureAnisotropic TextureSampler::getAnisotropic() const
{
    return m_anisotropicLevel;
}

} //namespace renderer
} //namespace v3d
