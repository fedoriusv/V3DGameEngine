#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class RenderTargetState;
    class GraphicsPipelineState;
    class UnorderedAccessBuffer;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineOutlineStage : public RenderPipelineStage
    {
    public:

        struct MappedData
        {
            u32* _ptr;
        };

        explicit RenderPipelineOutlineStage(RenderTechnique* technique) noexcept;
        ~RenderPipelineOutlineStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

    private:

        void createRenderTarget(renderer::Device* device, scene::SceneData& data);
        void destroyRenderTarget(renderer::Device* device, scene::SceneData& data);

        renderer::RenderTargetState* m_renderTarget;
        renderer::GraphicsPipelineState* m_pipeline;

        renderer::UnorderedAccessBuffer* m_readbackObjectID;
        MappedData m_mappedData;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::RenderPipelineOutlineStage::MappedData>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d