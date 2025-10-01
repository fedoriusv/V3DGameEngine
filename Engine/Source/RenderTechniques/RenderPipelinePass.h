#pragma once

#include "Common.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class RenderPipelinePass : u32
    {
        Opaque,
        SkinnedOpaque,
        MaskedOpaque,

        Transparency,
        SkinnedTransparency,

        Billboard,
        VFX,

        DirectionLight,
        PunctualLights,
        Shadowmap,

        Selected,
        Indicator,
        Debug,

        Custom,

        Count
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d