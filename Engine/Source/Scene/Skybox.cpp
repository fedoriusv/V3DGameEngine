#include "Skybox.h"

namespace v3d
{
namespace scene
{

Skybox::Skybox(renderer::Device* device) noexcept
    : m_device(device)
{
}

Skybox::Skybox(renderer::Device* device, const SkyboxHeader& header) noexcept
    : m_device(device)
    , m_header(header)
{
}

Skybox::~Skybox()
{
}

bool Skybox::load(const stream::Stream* stream, u32 offset)
{
    return false;
}

bool Skybox::save(stream::Stream* stream, u32 offset) const
{
    return false;
}

} //namespace scene
} //namespace v3d