#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineUIOverlayStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineUIOverlayStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineUIOverlayStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;
    };

} //namespace renderer
} //namespace v3d