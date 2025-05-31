#pragma once

#include "Common.h"
#include "Scene/Scene.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class RenderTechnique;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineStage
    {
    public:

        explicit RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept;
        virtual ~RenderPipelineStage();

        virtual void create(Device* device, scene::Scene::SceneData& data) = 0;
        virtual void destroy(Device* device, scene::Scene::SceneData& data) = 0;

        virtual void prepare(Device* device, scene::Scene::SceneData& data) = 0;
        virtual void execute(Device* device, scene::Scene::SceneData& data) = 0;

        virtual void changed(Device* device, scene::Scene::SceneData& data) {};

    protected:

        std::string m_id;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTechnique
    {
    public:

        void create(Device* device, scene::Scene::SceneData& data);
        void destroy(Device* device, scene::Scene::SceneData& data);

        void prepare(Device* device, scene::Scene::SceneData& data);
        void execute(Device* device, scene::Scene::SceneData& data);

        void changed(Device* device, scene::Scene::SceneData& data);

    protected:

        RenderTechnique() noexcept;
        virtual ~RenderTechnique();

        void addStage(const std::string& id, RenderPipelineStage* stage);

        struct Stage
        {
            std::string _id;
            RenderPipelineStage* _stage;
        };

        std::vector<Stage> m_stages;

        friend RenderPipelineStage;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d