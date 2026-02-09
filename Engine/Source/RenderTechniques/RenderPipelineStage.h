#pragma once

#include "Common.h"
#include "VertexFormats.h"

#include "Scene/Scene.h"
#include "Task/Task.h"
#include "Task/TaskScheduler.h"
#include "Memory/ThreadSafeAllocator.h"

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

        RenderPipelineStage* getStage(const std::string& id);

    protected:

        RenderTechnique() noexcept;
        virtual ~RenderTechnique();

        void addStage(const std::string& id, RenderPipelineStage* stage);
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

        virtual void onChanged(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame);

    protected:

        template<typename Func>
        void addRenderJob(const std::string& name, Func&& func, renderer::Device* device, const scene::SceneData& scene);

        std::string          m_id;
        RenderTechnique&     m_renderTechnique;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Func>
    inline void RenderPipelineStage::addRenderJob(const std::string& name, Func&& func, renderer::Device* device, const scene::SceneData& scene)
    {
        renderer::CmdListRender* cmdList = m_renderTechnique.acquireCmdList(device);
        u32 prevIndex = (scene.m_stateIndex + scene.m_frameState.size() - 1) % scene.m_frameState.size();

        //TODO: Hack to skip first render threads execution due empty data
        if (scene.m_frameState[prevIndex].m_frameResources.empty())
        {
            return;
        }

        task::Task* renderTask = new task::Task;
        renderTask->init(name, std::forward<Func>(func), device, cmdList, std::reference_wrapper<const scene::SceneData>(scene), std::reference_wrapper<const scene::FrameData>(scene.m_frameState[prevIndex]));

        m_renderTechnique.addRenderJob(device, cmdList, scene.m_taskWorker, renderTask);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d