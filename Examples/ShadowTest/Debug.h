#pragma once

#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/StreamBuffer.h"

#include "Scene/CameraHelper.h"
#include "Scene/ModelHelper.h"

namespace v3d
{
    struct DirectionLightDebug
    {
        void Init(renderer::CommandList* commandList, renderer::RenderTargetState* renderTarget);
        void Draw(renderer::CommandList* commandList, scene::Camera* camera, const core::Vector3D& angle, const core::Vector3D& size);
        void Free();

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        utils::IntrusivePointer<renderer::VertexStreamBuffer> m_Geometry;
        core::Vector4D m_LightColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    };

    struct LightDebug
    {
        void Init(v3d::renderer::CommandList* commandList, v3d::renderer::RenderTargetState* renderTarget);
        void Draw(v3d::renderer::CommandList* commandList, v3d::scene::Camera* camera, const v3d::core::Vector3D& light);
        void Free();

        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        v3d::scene::ModelHelper* m_Geometry = nullptr;
        v3d::core::Vector4D m_lightColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    };


} //namespace v3d