#pragma once

#include "Common.h"
#include "VertexFormats.h"
#include "Events/Game/GameEvent.h"
#include "Task/Task.h"
#include "Task/TaskScheduler.h"
#include "Thread/ThreadSafeAllocator.h"
#include "Scene/Scene.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdList;
    class RenderTargetState;
    class GraphicsPipelineState;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineStage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTechnique
    {
    public:

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        void submit(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

        void onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event);

        RenderPipelineStage* getStage(const std::string& id);

    protected:

        RenderTechnique() noexcept;
        virtual ~RenderTechnique();

        void addStage(const std::string& id, RenderPipelineStage* stage);

    private:

        void addRenderJob(renderer::Device* device, renderer::CmdListRender* cmd, task::TaskScheduler& worker, task::Task* renderTask);

        [[nodiscard]] renderer::CmdListRender* acquireCmdList(renderer::Device* device);

        struct Stage
        {
            std::string          _id;
            RenderPipelineStage* _stage;
        };

        std::vector<Stage> m_stages;
        std::vector<std::tuple<renderer::CmdListRender*, task::Task*>> m_dependencyList;
        std::queue<renderer::CmdListRender*> m_freeCmdList;

        using RenderJobFunc = std::function<void(renderer::Device*, renderer::CmdListRender*, const scene::SceneData&, const scene::FrameData&)>;
        std::vector<RenderJobFunc> m_batchJobs;
        std::vector<std::tuple<Object*, memory::MemoryLabel>> m_delayedDeleteList;

        friend RenderPipelineStage;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineStage
    {
    public:

        explicit RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept;
        virtual ~RenderPipelineStage();

        virtual void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

        virtual void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

        virtual void onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event);

        bool isEnabled() const;

    protected:

        template<typename Func>
        void addRenderJob(const std::string& name, Func&& func, renderer::Device* device, const scene::SceneData& scene, bool batch = false);

        std::string          m_id;
        RenderTechnique&     m_renderTechnique;
        bool                 m_enabled;
        bool                 m_created;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Func>
    inline void RenderPipelineStage::addRenderJob(const std::string& name, Func&& func, renderer::Device* device, const scene::SceneData& scene, bool batch)
    {
        if (batch)
        {
            m_renderTechnique.m_batchJobs.emplace_back(std::forward<Func>(func));
            return;
        }

        renderer::CmdListRender* cmdList = m_renderTechnique.acquireCmdList(device);
        scene::FrameData& frameData = scene.renderFrameData();

        task::Task* renderTask = new task::Task;
        if (m_renderTechnique.m_batchJobs.empty())
        {
            renderTask->init(name, std::forward<Func>(func), device, cmdList, std::reference_wrapper<const scene::SceneData>(scene), std::reference_wrapper<const scene::FrameData>(frameData));
        }
        else
        {
            m_renderTechnique.m_batchJobs.emplace_back(std::forward<Func>(func));

            std::vector<RenderTechnique::RenderJobFunc> tempBatchJobs;
            std::swap(tempBatchJobs, m_renderTechnique.m_batchJobs);

            auto batchFunc = [](std::vector<RenderTechnique::RenderJobFunc>& jobs, renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame)
                {
                    for (auto& job : jobs)
                    {
                        std::invoke(job, device, cmdList, scene, frame);
                    }
                };

            renderTask->init(batchFunc, tempBatchJobs, device, cmdList, std::reference_wrapper<const scene::SceneData>(scene), std::reference_wrapper<const scene::FrameData>(frameData));
        }

        m_renderTechnique.addRenderJob(device, cmdList, scene.m_taskWorker, renderTask);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d