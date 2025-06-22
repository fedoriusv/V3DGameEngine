#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace scene
{
    class ModelHandler;
} //namespace scene
namespace renderer
{
    class RenderPipelineGBufferStage : public RenderPipelineStage
    {
    public:

        struct MeshData
        {
        };

        explicit  RenderPipelineGBufferStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineGBufferStage();

        void create(Device* device, scene::SceneData& state) override;
        void destroy(Device* device, scene::SceneData& state) override;

        void prepare(Device* device, scene::SceneData& state) override;
        void execute(Device* device, scene::SceneData& state) override;

    private:

        void createRenderTarget(Device* device, scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_GBufferRenderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipeline;
    };

} //namespace renderer
} //namespace v3d