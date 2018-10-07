#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"
#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    struct RenderPassInfo
    {
        std::array<AttachmentDescription, k_maxFramebufferAttachments> _attachments; //32
        u32  _countColorAttachments   : 24;
        u32 _hasDepthStencilAttahment : 8;

    };

    class RenderPass : public utils::Observable
    {
    public:

        union RenderPassDescription
        {
            RenderPassInfo  _info;
            u32             _hash;
        };

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

        RenderPass* acquireRenderPass(const RenderPass::RenderPassDescription& desc)
        {
            const RenderPassInfo* pDesc = &desc._info;
            RenderPass* renderpass = m_context->createRenderPass(pDesc);

            renderpass->registerNotify(this);
            return renderpass;

            /*RenderPass* renderpass = nullptr;
            auto found = m_renderpasses.emplace(desc._hash, renderpass);
            if (!found.second)
            {
                renderpass = m_context->createRenderPass(&desc._info);
                found.first->second = renderpass;

                return renderpass;
            }

            return found.first->second;*/
        }

        void handleNotify(utils::Observable* ob) override
        {
            //TODO:
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

        Context* m_context;
        std::map<u32, RenderPass*> m_renderpasses;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
