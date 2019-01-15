#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class RenderPass;
    class Image;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Framebuffer base class. Render side
    */
    class Framebuffer : public utils::Observable
    {
    public:

        Framebuffer() noexcept;
        virtual ~Framebuffer();

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * FramebufferManager class
    */
    class FramebufferManager final : utils::Observer
    {
    public:

        FramebufferManager() = delete;

        explicit FramebufferManager(Context *context) noexcept;
        ~FramebufferManager();

        Framebuffer* acquireFramebuffer(const RenderPass* renderpass, const std::vector<Image*>& images, const core::Dimension2D& size);
        bool removeFramebuffer(const std::vector<Image*>& images);
        void clear();

        void handleNotify(utils::Observable* ob) override;

    private:

        Context* m_context;
        std::map<u32, Framebuffer*> m_framebuffers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
