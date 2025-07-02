#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace ui
{
    class WidgetHandler;
} // namespace ui
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineUIOverlayStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineUIOverlayStage(RenderTechnique* technique, ui::WidgetHandler* uiHandler) noexcept;
        ~RenderPipelineUIOverlayStage();

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace renderer
} // namespace v3d