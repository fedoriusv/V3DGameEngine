#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "BufferProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceCaps
    {
        u32 maxColorattachments = k_maxFramebufferAttachments;
        u32 maxVertexInputAttributes = k_maxVertexInputAttributes;
        u32 maxVertexInputBindings = k_maxVertexInputBindings;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
