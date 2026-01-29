#pragma once

#include "Common.h"
#include "VertexFormats.h"

#include "Scene/Scene.h"
#include "Task/Task.h"

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

    protected:

        RenderTechnique() noexcept;
        virtual ~RenderTechnique();

        void addStage(const std::string& id, RenderPipelineStage* stage);
        void addRenderJob(renderer::Device* device, renderer::CmdList* cmd, task::Task* renderTask);

        struct Stage
        {
            std::string          _id;
            RenderPipelineStage* _stage;
        };

        std::vector<Stage>              m_stages;

        task::TaskScheduler                                      m_renderWorker;
        std::vector<std::tuple<renderer::CmdList*, task::Task*>> m_dependencyList;

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

    protected:

        template<typename Func>
        void addRenderJob(const std::string& name, Func&& func, renderer::Device* device, renderer::CmdListRender* cmd, const scene::SceneData& scene, const scene::FrameData& frame);

        std::string          m_id;
        RenderTechnique&     m_renderTechnique;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Func>
    inline void RenderPipelineStage::addRenderJob(const std::string& name, Func&& func, renderer::Device* device, renderer::CmdListRender* cmd, const scene::SceneData& scene, const scene::FrameData& frame)
    {
        task::Task* renderTask = new task::Task;
        renderTask->init(name, std::forward<Func>(func), cmd, scene, frame);

        m_renderTechnique.addRenderJob(device, cmd, renderTask);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d