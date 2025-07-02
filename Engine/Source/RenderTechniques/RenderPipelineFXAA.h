#pragma once

#include "Common.h"

#include "RenderPipelineStage.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineFXAAStage : public RenderPipelineStage
    {
    public:

        explicit RenderPipelineFXAAStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineFXAAStage();

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_renderTarget;
        renderer::GraphicsPipelineState* m_pipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d