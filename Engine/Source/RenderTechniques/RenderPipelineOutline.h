#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineOutlineStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineOutlineStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineOutlineStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;
    };

} //namespace renderer
} //namespace v3d