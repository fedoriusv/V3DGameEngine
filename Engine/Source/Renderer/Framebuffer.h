#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"
#include "Utils/Logger.h"
#include "Context.h"
#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPass;
    class Image;

    class Framebuffer : public utils::Observable
    {
    public:

        Framebuffer() {};
        virtual ~Framebuffer() {};

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class FramebufferManager final : utils::Observer
    {
    public:

        FramebufferManager(Context *context)
            : m_context(context)
        {
        }

        ~FramebufferManager()
        {
            FramebufferManager::clear();
        }

        Framebuffer* acquireFramebuffer(const RenderPass* renderpass, const std::vector<Image*>& images, const core::Dimension2D& size)
        {
            u32 hash = crc32c::Crc32c((char*)images.data(), images.size() * sizeof(Image*));

            Framebuffer* framebuffer = nullptr;
            auto found = m_framebuffers.emplace(hash, framebuffer);
            if (found.second)
            {
                framebuffer = m_context->createFramebuffer(images, size);
                if (!framebuffer->create(renderpass))
                {
                    m_framebuffers.erase(hash);

                    ASSERT(false, "can't create framebuffer");
                    return nullptr;
                }
                found.first->second = framebuffer;
                framebuffer->registerNotify(this);

                return framebuffer;
            }

            return found.first->second;
        }

        bool removeFramebuffer(Framebuffer* framebuffer)
        {
            /*if (!m_framebuffers.erase(hash))
            {
                LOG_DEBUG("FramebufferManager renderpass not found");
                ASSERT(false, "renderpass");
                return false;
            }*/

            return true;
        }

        void handleNotify(utils::Observable* ob) override
        {
            LOG_DEBUG("RenderPassManager renderpass %x has been deleted", ob);
            //m_renderpasses.erase(ob);
        }

        void clear()
        {
            for (auto& renderpass : m_framebuffers)
            {
                renderpass.second->destroy();
                renderpass.second->notifyObservers();

                delete renderpass.second;
            }
            m_framebuffers.clear();
        }


    private:

        Context* m_context;
        std::map<u32, Framebuffer*> m_framebuffers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
