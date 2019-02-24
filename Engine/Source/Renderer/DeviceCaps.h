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
        enum TilingType
        {
            TilingType_Optimal = 0,
            TilingType_Linear = 1,

            TilingType_Count
        };

        u32 maxColorattachments = k_maxFramebufferAttachments;
        u32 maxVertexInputAttributes = k_maxVertexInputAttributes;
        u32 maxVertexInputBindings = k_maxVertexInputBindings;

        bool invertZ = true;

        struct ImageFormatSupport
        {
            bool _supportAttachment;
            bool _supportSampled;
        };

        const ImageFormatSupport& getImageFormatSupportInfo(Format format, TilingType type)
        {
            return m_imageFormatSupport[format][type];
        }

    protected:

        ImageFormatSupport m_imageFormatSupport[Format::Format_Count][TilingType::TilingType_Count];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
