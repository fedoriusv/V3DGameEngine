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
    class RenderPassManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderPass base class. Render side
    */
    class RenderPass : public RenderObject<RenderPass>, public utils::Observable
    {
    public:

        struct RenderPassInfo
        {
            RenderPassInfo() noexcept
                : _tracker(nullptr)
            {
            }

            union RenderPassDesc
            {
                RenderPassDesc() noexcept
                {
                    memset(this, 0, sizeof(RenderPassDesc));
                }

                RenderPassDescription _desc;
                u32                   _hash;
            };
            RenderPassDesc             _value;
            ObjectTracker<RenderPass>* _tracker;
        };

        RenderPass() noexcept;
        virtual ~RenderPass();

        virtual bool create() = 0;
        virtual void destroy() = 0;

    private:

        u32 m_key;

        friend RenderPassManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderPassManager class
    */
    class RenderPassManager final : utils::Observer
    {
    public:

        RenderPassManager() = delete;
        RenderPassManager(const RenderPassManager&) = delete;

        explicit RenderPassManager(Context *context) noexcept;
        ~RenderPassManager();

        RenderPass* acquireRenderPass(const RenderPassDescription& renderPassInfo);
        bool removeRenderPass(const RenderPass* renderPass);
        void clear();

        void handleNotify(utils::Observable* ob) override;

    private:

        Context* m_context;
        std::map<u32, RenderPass*> m_renderPassList;
        //std::unordered_map<u32, RenderPass*> m_renderPasses; TODO
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
