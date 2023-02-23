#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Image;
    class Context;
    class RenderPass;
    class FramebufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Framebuffer base class. Render side
    */
    class Framebuffer : public RenderObject<Framebuffer>, public utils::Observable
    {
    public:

        /**
        * @brief RegionInfo struct
        */
        struct ClearValueInfo
        {
            TargetRegion                _region;
            std::vector<core::Vector4D> _color;
            f32                         _depth = 0.f;
            u32                         _stencil = 0;
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

        Framebuffer() noexcept = default;
        virtual ~Framebuffer() = default;

        virtual bool create(const RenderPass* pass) = 0;
        virtual void destroy() = 0;

    private:

        struct FramebufferDescription
        {
            FramebufferDescription() noexcept;
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

        explicit FramebufferManager(Context *context) noexcept;
        ~FramebufferManager();

        std::tuple<Framebuffer*, bool> acquireFramebuffer(const RenderPass* renderpass, const std::vector<Image*>& images, const core::Dimension2D& area);
        bool removeFramebuffer(Framebuffer* framebufer);
        void clear();

        void handleNotify(const utils::Observable* object, void* msg) override;

    private:

        FramebufferManager() = delete;
        FramebufferManager(const FramebufferManager&) = delete;

        Context* const m_context;
        std::unordered_map<Framebuffer::FramebufferDescription, Framebuffer*, Framebuffer::FramebufferDescription::Hash, Framebuffer::FramebufferDescription::Compare> m_framebufferList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
