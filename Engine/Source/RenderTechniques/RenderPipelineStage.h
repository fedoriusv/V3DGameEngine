#pragma once

#include "Common.h"
#include "VertexFormats.h"
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

        virtual void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

        virtual void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

    protected:

        std::string m_id;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTechnique
    {
    public:

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame);

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame);

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