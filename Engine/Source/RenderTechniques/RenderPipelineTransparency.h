#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineTransparencyStage : public RenderPipelineStage
    {
    public:

        struct MeshData
        {
        };

        explicit  RenderPipelineTransparencyStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineTransparencyStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_transparencyRenderTarget;
        std::vector<v3d::renderer::GraphicsPipelineState*> m_pipeline;
    };

} //namespace renderer
} //namespace v3d