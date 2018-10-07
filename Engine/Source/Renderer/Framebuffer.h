#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;
    class RenderPass;

    class Framebuffer : public utils::Observable
    {
    public:

        struct FramebufferDescription
        {
            core::Dimension2D                  _size;
            std::vector<AttachmentDescription> _attachments;
        };

        Framebuffer() {};
        virtual ~Framebuffer() {};

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
