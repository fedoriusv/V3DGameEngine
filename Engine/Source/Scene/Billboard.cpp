#include "Billboard.h"

namespace v3d
{
namespace scene
{

Billboard::Billboard(renderer::Device* device) noexcept
    : m_device(device)
{
}

Billboard::Billboard(renderer::Device* device, const BillboardHeader& header) noexcept
    : m_device(device)
    , m_header(header)
{
}

Billboard::~Billboard()
{
}

bool Billboard::load(const stream::Stream* stream, u32 offset)
{
    return false;
}

bool Billboard::save(stream::Stream* stream, u32 offset) const
{
    return false;
}

} //namespace scene
} //namespace v3d