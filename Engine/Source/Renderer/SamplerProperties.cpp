#include "SamplerProperties.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

SamplerDescription::SamplerDescription() noexcept
    : _hash(0)
    , _dirty(true)
{
    memset(&_desc, 0, sizeof(SamplerDescription::SamplerDesc));
}

bool SamplerDescription::operator==(const SamplerDescription& other) const
{
    if (this == &other)
    {
        return true;
    }

    return memcmp(&_desc, &other._desc, sizeof(SamplerDescription::SamplerDesc)) == 0;
}

void SamplerDescription::dirty()
{
    _dirty = true;
}

void SamplerDescription::recalculateHash() const
{
    _hash = crc32c::Crc32c(reinterpret_cast<const u8*>(&_desc), sizeof(GraphicsPipelineStateDescription));
    _dirty = false;
}

u64 SamplerDescription::Hash::operator()(const SamplerDescription& other) const
{
    if (other._dirty)
    {
        other.recalculateHash();
    }

    return other._hash;
}

bool SamplerDescription::Compare::operator()(const SamplerDescription& op1, const SamplerDescription& op2) const
{
    return op1 == op2;
}

} //namespace renderer
} //namespace v3d