#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineDeferredLightingStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineDeferredLightingStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineDeferredLightingStage();

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_deferredRenderTarget;
        v3d::renderer::GraphicsPipelineState* m_pipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d