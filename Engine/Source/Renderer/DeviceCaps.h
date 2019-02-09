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

        bool invertZ = true;

        struct ImageFormatSupport
        {
            bool _tilingOptimal;
            bool _tilingLinear;
            bool _supportAttachment;
            bool _supportSampled;
        };

        const ImageFormatSupport& getIFormatSupportInfo(Format format)
        {
            return m_imageFormatSupport[format];
        }

    protected:

        ImageFormatSupport m_imageFormatSupport[Format::Format_Count];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
