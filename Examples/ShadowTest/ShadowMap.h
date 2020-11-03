#pragma once

#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"

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
        void Update(f32 dt, const scene::CameraHelper* worldCamera);
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
        core::Rect32 m_Size = { 0, 0, 2048, 2048 };
        s32 m_Scale = 5;

        core::Matrix4D m_LightSpaceMatrix;
    };

    class ShadowMappingPoint
    {
    public:

        ShadowMappingPoint(renderer::CommandList* cmdList) noexcept;

        void Init(const renderer::VertexInputAttribDescription& desc);
        void Update(f32 dt, const core::Vector3D& position);
        void Draw(scene::ModelHelper* geometry, const scene::Transform& transform);
        void Free();

    private:

        renderer::CommandList* const m_CmdList;

        utils::IntrusivePointer<renderer::RenderTargetState> m_RenderTarget[6];
        utils::IntrusivePointer<renderer::TextureCube> m_DepthAttachment;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        scene::CameraHelper* m_ShadowCamera = nullptr;
        core::Dimension2D m_Size = { 1024, 1024 };

        std::array<core::Matrix4D, 6> m_LightSpaceMatrices;
    };

} //namespace v3d