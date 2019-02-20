#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "TextureProperties.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class RenderPass;
    class Image;
    class FramebufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Framebuffer base class. Render side
    */
    class Framebuffer : public RenderObject<Framebuffer>, public utils::Observable
    {
    public:

        struct ClearValueInfo
        {
            core::Dimension2D           _size;
            std::vector<core::Vector4D> _color;
            f32                         _depth;
            u32                         _stencil;
        };

        struct FramebufferInfo
        {
            std::vector<Image*>         _images;
            ClearValueInfo              _clearInfo;

            ObjectTracker<Framebuffer>* _tracker;
        };

        Framebuffer() noexcept;
        virtual ~Framebuffer();

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;

    private:

        u32 m_key;

        friend FramebufferManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * FramebufferManager class
    */
    class FramebufferManager final : utils::Observer
    {
    public:

        FramebufferManager() = delete;
        FramebufferManager(const FramebufferManager&) = delete;

        explicit FramebufferManager(Context *context) noexcept;
        ~FramebufferManager();

        Framebuffer* acquireFramebuffer(const RenderPass* renderpass, const Framebuffer::FramebufferInfo& framebufferInfo);
        bool removeFramebuffer(Framebuffer* framebufer);
        void clear();

        void handleNotify(utils::Observable* object) override;

    private:

        Context* m_context;
        std::map<u32, Framebuffer*> m_framebufferList;
        //std::unordered_map<u32, Framebuffer*> m_framebufferList; TODO
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
