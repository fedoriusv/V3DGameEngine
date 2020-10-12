#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "TextureProperties.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;
    class RenderPass;
    class Image;
    class FramebufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Framebuffer base class. Render side
    */
    class Framebuffer : public RenderObject<Framebuffer>, public utils::Observable
    {
    public:

        /**
        * @brief ClearValueInfo struct
        */
        struct ClearValueInfo
        {
            ClearValueInfo()
                : _depth(0.0f)
                , _stencil(0U)
            {
            }

            void clear()
            {
                _size = core::Dimension2D(0, 0);
                _color.clear();
                _depth = 0.0f;
                _stencil = 0U;
            }

            core::Dimension2D           _size;
            std::vector<core::Vector4D> _color;
            f32                         _depth;
            u32                         _stencil;
        };

        /**
        * @brief FramebufferInfo struct
        */
        struct FramebufferInfo
        {
            FramebufferInfo() noexcept
                : _tracker(nullptr)
            {
            }

            std::vector<Image*>         _images;
            ClearValueInfo              _clearInfo;

            ObjectTracker<Framebuffer>* _tracker;
        };

        Framebuffer() noexcept;
        virtual ~Framebuffer();

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;

    private:

        struct FramebufferDescription
        {
            FramebufferDescription() = default;
            FramebufferDescription(u64 size) noexcept;
            bool operator==(const FramebufferDescription& desc) const;

            struct Hash
            {
                u64 operator()(const FramebufferDescription& desc) const;
            };

            struct Compare
            {
                bool operator()(const FramebufferDescription& op1, const FramebufferDescription& op2) const;
            };

            u64 _hash;
            std::vector<std::tuple<const Image*, s32>> _imagesDesc;
        };

        FramebufferDescription m_desc;
        friend FramebufferManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief FramebufferManager class
    */
    class FramebufferManager final : utils::Observer
    {
    public:

        FramebufferManager() = delete;
        FramebufferManager(const FramebufferManager&) = delete;

        explicit FramebufferManager(Context *context) noexcept;
        ~FramebufferManager();

        std::tuple<Framebuffer*, bool> acquireFramebuffer(const RenderPass* renderpass, const std::vector<Image*>& images, const core::Dimension2D& area);
        bool removeFramebuffer(Framebuffer* framebufer);
        void clear();

        void handleNotify(utils::Observable* object) override;

    private:

        Context* m_context;
        //std::map<u32, Framebuffer*> m_framebufferList;
        std::unordered_map<Framebuffer::FramebufferDescription, Framebuffer*, Framebuffer::FramebufferDescription::Hash, Framebuffer::FramebufferDescription::Compare> m_framebufferList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
