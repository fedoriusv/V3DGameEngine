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

    class Context;
    class RenderPassManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderPass base class. Render side
    */
    class RenderPass : public RenderObject<RenderPass>, public utils::Observable
    {
    public:

        /**
        * @brief RenderPassInfo struct
        */
        struct RenderPassInfo
        {
            RenderPassInfo() noexcept
                : _tracker(nullptr)
            {
            }

            RenderPassDescription       _desc;
            ObjectTracker<RenderPass>*  _tracker;
        };

        explicit RenderPass(const RenderPassDescription& desc) noexcept;
        virtual ~RenderPass();

        virtual bool create() = 0;
        virtual void destroy() = 0;

        const RenderPassDescription::RenderPassDesc& getDescription() const;

    protected:

        RenderPass() = delete;
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        const RenderPassDescription m_desc;
        friend RenderPassManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderPassManager class
    */
    class RenderPassManager final : utils::Observer
    {
    public:

        explicit RenderPassManager(Context *context) noexcept;
        ~RenderPassManager();

        [[nodiscard]] RenderPass* acquireRenderPass(const RenderPassDescription& renderPassInfo);
        bool removeRenderPass(const RenderPass* renderPass);
        void clear();

        void handleNotify(const utils::Observable* object, void* msg) override;

    private:

        RenderPassManager() = delete;
        RenderPassManager(const RenderPassManager&) = delete;
        RenderPassManager& operator=(const RenderPassManager&) = delete;

        Context* const m_context;
        std::unordered_map<RenderPassDescription, RenderPass*, RenderPassDescription::Hash, RenderPassDescription::Compare> m_renderPassList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
