#pragma once

#include "Common.h"
#include "RenderPipelineStage.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
    class Texture2D;
    class RenderTargetState;
    class GraphicsPipelineState;
} // namespace renderer
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ModelHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineShadowStage : public RenderPipelineStage
    {
    public:

        explicit  RenderPipelineShadowStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept;
        ~RenderPipelineShadowStage();

        void create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;
        void execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame) override;

        void onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event) override;

        struct PipelineData
        {
            PipelineData(thread::ThreadSafeAllocator* allocator);

            math::Dimension2D                                       _shadowSize;
            std::array<f32, k_maxShadowmapCascadeCount>             _directionLightCascadeSplits;
            std::array<math::Matrix4D, k_maxShadowmapCascadeCount>  _directionLightSpaceMatrix;
            std::array<u32, k_maxPunctualShadowmapCount>            _punctualLightsFlags;
            std::array<std::tuple<std::array<math::Matrix4D, 6>, math::Vector3D, math::float2, u32>, k_maxPunctualShadowmapCount> _punctualLightsData;

            thread::ThreadSafeAllocator* _allocator;
        };

    private:

        void createRenderTarget(renderer::Device* device, SceneData& scene);
        void destroyRenderTarget(renderer::Device* device, SceneData& scene);

        void createPipelines(renderer::Device* device, scene::SceneData& scene);
        void destroyPipelines(renderer::Device* device, scene::SceneData& scene);

        static void calculateShadowCascades(const SceneData& data, const math::Vector3D& lightDirection, u32 cascadeCount, math::Matrix4D* lightSpaceMatrixOut, f32* cascadeSplitsOut);
        static void calculateShadowViews(const math::Vector3D& position, f32 nearPlane, f32 farPlane, u32 viewsMask, std::array<math::Matrix4D, 6>& lightSpaceMatrix);

        scene::ModelHandler* const                m_modelHandler;
        renderer::SamplerState*                   m_shadowSamplerState;

        struct MaterialCascadeShadowsParameters
        {
            SHADER_PARAMETER(cb_DirectionShadowBuffer);
        };

        renderer::RenderTargetState*              m_cascadeRenderTarget;
        renderer::Texture2D*                      m_cascadeTextureArray;
        renderer::GraphicsPipelineState*          m_cascadeShadowPipeline;
        MaterialCascadeShadowsParameters          m_cascadeShadowParameters;

        struct MaterialPointShadowsParameters
        {
            SHADER_PARAMETER(cb_PunctualShadowBuffer);
        };

        renderer::RenderTargetState*              m_punctualShadowRenderTarget;
        renderer::Texture2D*                      m_punctualShadowTextureArray;
        renderer::GraphicsPipelineState*          m_punctualShadowPipeline;
        MaterialPointShadowsParameters            m_punctualShadowParameters;

        struct MaterialScreenSpaceShadowsParameters
        {
            SHADER_PARAMETER(cb_Viewport);
            SHADER_PARAMETER(s_SamplerState);
            SHADER_PARAMETER(s_ShadowSamplerState);
            SHADER_PARAMETER(t_TextureDepth);
            SHADER_PARAMETER(t_TextureNormals);
            SHADER_PARAMETER(cb_ShadowmapBuffer);
            SHADER_PARAMETER(t_DirectionCascadeShadows);
        };

        renderer::RenderTargetState*              m_SSShadowsRenderTarget;
        renderer::GraphicsPipelineState*          m_SSShadowsPipeline;
        MaterialScreenSpaceShadowsParameters      m_SSCascadeShadowParameters;

    };

} // namespace scene

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::RenderPipelineShadowStage::PipelineData>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d