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
    static_assert(sizeof(SamplerDescription::SamplerDesc) == 24, "wrong size");
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
    _hash = crc32c::Crc32c(reinterpret_cast<const u8*>(&_desc), sizeof(SamplerDescription::SamplerDesc));
    _dirty = false;
}

u32 SamplerDescription::Hash::operator()(const SamplerDescription& desc) const
{
    if (desc._dirty)
    {
        desc.recalculateHash();
    }

    ASSERT(desc._hash != 0, "must be not 0");
    return desc._hash;
}

bool SamplerDescription::Compare::operator()(const SamplerDescription& op1, const SamplerDescription& op2) const
{
    ASSERT(op1._hash != 0 && op2._hash != 0, "must be not 0");
    if (op1._hash != op2._hash)
    {
        return false;
    }

    return op1 == op2;
}

} //namespace renderer
} //namespace v3d