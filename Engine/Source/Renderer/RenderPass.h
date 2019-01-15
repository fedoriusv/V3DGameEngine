#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderPass base class. Render side
    */
    class RenderPass : public utils::Observable
    {
    public:

        struct RenderPassInfo
        {
            RenderPassInfo()
            {
                _attachments.fill(AttachmentDescription());
                _countColorAttachments = 0;
                _hasDepthStencilAttahment = false;
            }

            std::array<AttachmentDescription, k_maxFramebufferAttachments> _attachments; //32
            u32  _countColorAttachments : 24;
            u32 _hasDepthStencilAttahment : 8;

        };

        struct ClearValueInfo
        {
            core::Dimension2D           _size;
            std::vector<core::Vector4D> _color;
            f32                         _depth;
            u32                         _stencil;

        };

        RenderPass() noexcept;
        virtual ~RenderPass();

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderPassManager class
    */
    class RenderPassManager final : utils::Observer
    {
    public:

        RenderPassManager() = delete;

        explicit RenderPassManager(Context *context) noexcept;
        ~RenderPassManager();

        RenderPass* acquireRenderPass(const RenderPass::RenderPassInfo& desc);
        bool removeRenderPass(const RenderPass::RenderPassInfo& desc);
        void clear();

        void handleNotify(utils::Observable* ob) override;

    private:

        union RenderPassDescription
        {
            RenderPassDescription() {}

            RenderPass::RenderPassInfo  _info;
            u32                         _hash;
        };

        Context* m_context;
        std::map<u32, RenderPass*> m_renderpasses;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
