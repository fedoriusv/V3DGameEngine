#include "TextureProperties.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

RenderPassDescription::RenderPassDescription() noexcept
{
    static_assert(sizeof(RenderPassDescription) == 40, "wrong size");
    memset(&_desc, 0, sizeof(RenderPassDescription::RenderPassDesc));
}

bool RenderPassDescription::operator==(const RenderPassDescription& other) const
{
    if (this == &other)
    {
        return true;
    }

    return memcmp(&_desc, &other._desc, sizeof(RenderPassDescription::RenderPassDesc)) == 0;
}

u32 RenderPassDescription::countActiveViews(u32 viewsMask)
{
    u32 countActiveLayers = 0;
    for (u32 i = 0; i < std::numeric_limits<u32>::digits; ++i)
    {
        if ((viewsMask >> i) & 0x1)
        {
            ++countActiveLayers;
        }

    }

    return countActiveLayers;
}

bool RenderPassDescription::isActiveViewByIndex(u32 viewsMask, u32 index)
{
    return (viewsMask >> index) & 0x1;
}

u32 RenderPassDescription::Hash::operator()(const RenderPassDescription& desc) const
{
    return crc32c::Crc32c(reinterpret_cast<const u8*>(&desc), sizeof(RenderPassDescription::RenderPassDesc));
}

bool RenderPassDescription::Compare::operator()(const RenderPassDescription& op1, const RenderPassDescription& op2) const
{
    return op1 == op2;
}

} //namespace renderer
} //namespace v3d