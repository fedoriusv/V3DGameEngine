#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Buffer.h"

namespace v3d
{
namespace renderer
{
    class RenderPipelineOutlineStage : public RenderPipelineStage
    {
    public:

        struct MappedData
        {
            u32* _ptr;
        };

        explicit RenderPipelineOutlineStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineOutlineStage();

        void create(Device* device, scene::Scene::SceneData& state) override;
        void destroy(Device* device, scene::Scene::SceneData& state) override;

        void prepare(Device* device, scene::Scene::SceneData& state) override;
        void execute(Device* device, scene::Scene::SceneData& state) override;

        void changed(Device* device, scene::Scene::SceneData& data) override;

    private:

        void createRenderTarget(Device* device, scene::Scene::SceneData& data);
        void destroyRenderTarget(Device* device, scene::Scene::SceneData& data);

        renderer::RenderTargetState* m_renderTarget;
        renderer::SamplerState* m_sampler;
        renderer::GraphicsPipelineState* m_pipeline;

        renderer::UnorderedAccessBuffer* m_readbackObjectID;
        MappedData m_mappedData;

    };

} //namespace renderer

    template<>
    struct TypeOf<renderer::RenderPipelineOutlineStage::MappedData>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

} //namespace v3d