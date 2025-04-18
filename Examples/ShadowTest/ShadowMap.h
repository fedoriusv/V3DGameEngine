#pragma once

#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"

#include "Scene/CameraHelper.h"
#include "Scene/ModelHelper.h"

namespace v3d
{
    class ShadowMapping
    {
    public:

        ShadowMapping(renderer::CommandList* cmdList) noexcept;

        void Init(const renderer::VertexInputAttribDescription& desc);
        void Update(f32 dt, const core::Vector3D& position, const core::Vector3D& target);
        void Draw(scene::ModelHelper* geometry, const scene::Transform& transform);
        void Free();

        const core::Matrix4D& GetLightSpaceMatrix() const;
        const renderer::Texture2D* GetDepthMap() const;

    private:

        renderer::CommandList* const m_CmdList;

        utils::IntrusivePointer<renderer::RenderTargetState> m_RenderTarget;
        utils::IntrusivePointer<renderer::Texture2D> m_DepthAttachment;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        scene::CameraHelper* m_ShadowCamera = nullptr;
        const core::Rect32 m_Size = { 0, 0, 2048, 2048 };
        const s32 m_Extent = 4;

        core::Matrix4D m_LightSpaceMatrix;
    };


    class CascadedShadowMapping
    {
    public:

        CascadedShadowMapping(renderer::CommandList* cmdList) noexcept;

        void Init(const renderer::VertexInputAttribDescription& desc);
        void Update(f32 dt, const scene::Camera& camera, const core::Vector3D& position, const core::Vector3D& target);
        void Draw(scene::ModelHelper* geometry, const scene::Transform& transform);
        void Free();

        const renderer::Texture2DArray* GetDepthMap() const;
        const std::vector<f32>& GetCascadeSplits() const;
        const std::vector<core::Matrix4D>& GetLightSpaceMatrix() const;

        static const u32 s_CascadeCount = 4;

    private:

        void CalculateShadowCascades(const v3d::scene::Camera& camera, const core::Vector3D& light);

        renderer::CommandList* const m_CmdList;

        std::vector<utils::IntrusivePointer<renderer::RenderTargetState>> m_RenderTargets;
        utils::IntrusivePointer<renderer::Texture2DArray> m_DepthAttachment;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        const core::Rect32 m_Size = { 0, 0, 2048, 2048 };

        std::vector<core::Matrix4D> m_LightSpaceMatrices;
        std::vector<f32> m_CascadeSplits;
    };

    class ShadowMappingPoint
    {
    public:

        ShadowMappingPoint(renderer::CommandList* cmdList) noexcept;

        void Init(const renderer::VertexInputAttribDescription& desc);
        void Update(f32 dt, const core::Vector3D& position);
        void Draw(scene::ModelHelper* geometry, const scene::Transform& transform);
        void Free();

        const renderer::TextureCube* GetDepthMap() const;
        const scene::Camera& GetCamera() const;

    private:

        void PrepareShadowMap(const renderer::VertexInputAttribDescription& desc);
        void PrepareMuiltiviewShadowMap(const renderer::VertexInputAttribDescription& desc);

        renderer::CommandList* const m_CmdList;

        static const u32 s_PointSidesCount = 6;

        utils::IntrusivePointer<renderer::RenderTargetState> m_RenderTarget[s_PointSidesCount];
        utils::IntrusivePointer<renderer::TextureCube> m_DepthAttachment;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        scene::CameraHelper* m_ShadowCamera = nullptr;
        core::Dimension2D m_Size = { 2048, 2048 };

        std::array<core::Matrix4D, s_PointSidesCount> m_LightSpaceMatrices;

        bool m_UseMultiview = true;

    };

} //namespace v3d