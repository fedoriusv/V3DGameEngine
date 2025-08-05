#include "Billboard.h"

namespace v3d
{
namespace scene
{

Billboard::Billboard() noexcept
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