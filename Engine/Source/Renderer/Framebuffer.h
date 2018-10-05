#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    class Framebuffer
    {
    public:

        struct FramebufferDescription
        {
            core::Dimension2D                  _size;
            std::vector<AttachmentDescription> _attachments;
        };

        Framebuffer() {};
        virtual ~Framebuffer() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class FramebufferManager final
    {
    public:

        FramebufferManager(Context *context) 
        : m_context(context)
        {
        };

        ~FramebufferManager() {};

        Framebuffer* acquireFramebuffer(const Framebuffer::FramebufferDescription& desc)
        {

            Framebuffer* framebuffer = nullptr;
            auto found = m_framebuffers.emplace(desc, framebuffer);
            if (!found.second)
            {
                framebuffer = m_context->createFramebuffer(desc);

                //command

                found.first->first = desc;
                found.first->second = framebuffer;

                return framebuffer;
            }

            return found.first->second;
        }


        void clear()
        {
            for (auto& framebuffer : m_framebuffers)
            {
                framebuffer.second->destroy();
                framebuffer->notifyObservers();

                delete framebuffer.second;
            }
            m_framebuffers.clear();
        }


    private:

        Context* m_context;
        std::map<FramebufferDesc, Framebuffer*> m_framebuffers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
