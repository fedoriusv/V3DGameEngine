#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"
#include "Utils/Logger.h"
#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    struct RenderPassInfo
    {
        RenderPassInfo()
        {
            _attachments.fill(AttachmentDescription());
            _countColorAttachments = 0;
            _hasDepthStencilAttahment = false;
        }

        std::array<AttachmentDescription, k_maxFramebufferAttachments> _attachments; //32
        u32  _countColorAttachments   : 24;
        u32 _hasDepthStencilAttahment : 8;

    };

    struct ClearValueInfo
    {
        core::Dimension2D           _size;
        std::vector<core::Vector4D> _color;
        f32                         _depth;
        u32                         _stencil;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPass : public utils::Observable
    {
    public:

        RenderPass() {};
        virtual ~RenderPass() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPassManager final : utils::Observer
    {
    public:

        RenderPassManager(Context *context)
            : m_context(context)
        {
        };

        ~RenderPassManager() 
        {
            RenderPassManager::clear();
        };

        RenderPass* acquireRenderPass(const RenderPassInfo& desc)
        {
            RenderPassDescription pDesc;
            pDesc._info = desc;

            RenderPass* renderpass = nullptr;
            auto found = m_renderpasses.emplace(pDesc._hash, renderpass);
            if (found.second)
            {
                renderpass = m_context->createRenderPass(&pDesc._info);
                if (!renderpass->create())
                {
                    m_renderpasses.erase(pDesc._hash);

                    ASSERT(false, "can't create renderpass");
                    return nullptr;
                }
                found.first->second = renderpass;
                renderpass->registerNotify(this);

                return renderpass;
            }

            return found.first->second;
        }

        bool removeRenderPass(const RenderPassInfo& desc)
        {
            RenderPassDescription pDesc;
            pDesc._info = desc;

            if (!m_renderpasses.erase(pDesc._hash))
            {
                LOG_DEBUG("RenderPassManager renderpass not found");
                ASSERT(false, "renderpass");
                return false;
            }

            return true;
        }

        void handleNotify(utils::Observable* ob) override
        {
            LOG_DEBUG("RenderPassManager renderpass %x has been deleted", ob);
            //m_renderpasses.erase(ob);
        }

        void clear()
        {
            for (auto& renderpass : m_renderpasses)
            {
                renderpass.second->destroy();
                renderpass.second->notifyObservers();

                delete renderpass.second;
            }
            m_renderpasses.clear();
        }


    private:

        union RenderPassDescription
        {
            RenderPassDescription() {}

            RenderPassInfo  _info;
            u32             _hash;
        };

        Context* m_context;
        std::map<u32, RenderPass*> m_renderpasses;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
