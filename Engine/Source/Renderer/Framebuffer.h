#pragma once

#include "Common.h"
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

        Framebuffer() {};
        virtual ~Framebuffer() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class FramebufferManager final
    {
    public:

        struct FramebufferDesc
        {
            FramebufferDesc& operator=(const FramebufferDesc&)
            {
                return *this;
            }
        };

        FramebufferManager(Context *context) 
        : m_context(context)
        {
        };

        ~FramebufferManager() {};

        Framebuffer* acquireFramebuffer(const FramebufferDesc& desc)
        {

            Framebuffer* framebuffer = nullptr;
            auto found = m_framebuffers.emplace(desc, framebuffer);
            if (!found.second)
            {
                framebuffer = m_context->createFramebuffer();

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
