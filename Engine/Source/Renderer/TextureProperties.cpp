#include "TextureProperties.h"
#include "Texture.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{
TextureView::TextureView(const Texture* texture, u32 layer, u32 mip) noexcept
    : _texture(texture)
    , _baseLayer(layer)
    , _layers(1)
    , _baseMip(mip)
    , _mips(1)
{
    ASSERT(texture, "nullptr");
    if (layer == k_generalLayer)
    {
        _baseLayer = 0;
        _layers = _texture->getLayersCount();
    }

    if (mip == k_allMipmapsLevels)
    {
        _baseMip = 0;
        _mips = texture->getMipmapsCount();
    }
}

TextureView::TextureView(const Texture* texture, u32 baseLayer, u32 layers, u32 baseMip, u32 mips) noexcept
    : _texture(texture)
    , _baseLayer(baseLayer)
    , _layers(layers)
    , _baseMip(baseMip)
    , _mips(mips)
{
    ASSERT(texture, "nullptr");
}

TargetRegion::TargetRegion(u32 width, u32 height) noexcept
    : _size(0, 0, width, height)
    , _baseLayer(0)
    , _layers(1)
{
}

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