#pragma once

#include "Common.h"
#include "Scene/Transform.h"
#include "Scene/Camera/CameraHandler.h"

#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "RenderTechniques/RenderObjectTracker.h"

#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{
    struct ViewportState
    {
        alignas(16) struct ViewportBuffer
        {
            math::Matrix4D projectionMatrix;
            math::Matrix4D viewMatrix;
            math::Matrix4D prevProjectionMatrix;
            math::Matrix4D prevViewMatrix;
            math::float2   cameraJitter;
            math::float2   prevCameraJitter;
            math::float4   cameraPosition;
            math::float4   random;
            math::float2   viewportSize;
            math::float2   clipNearFar;
            math::float2   cursorPosition;
            u64            time;

        }                       _viewportBuffer;

        math::Dimension2D       _viewpotSize;
        scene::CameraHandler*   _camera;
    };

    struct DrawInstanceData
    {
        renderer::IndexBuffer* m_IdxBuffer;
        renderer::VertexBuffer* m_VtxBuffer;

        scene::Transform m_transform;
        scene::Transform m_prevTransform;

        renderer::SamplerState* m_sampler;
        renderer::Texture2D* m_albedo;
        renderer::Texture2D* m_normals;
        renderer::Texture2D* m_material;
        math::float4 m_tint;


        utils::StringID m_stageID;
        u64 m_pipelineID;
        u64 m_objectID;
    };

    struct EditorState
    {
        u32 selectedObjectID;
    };

    struct RenderState
    {
        renderer::CmdListRender* m_cmdList;
    };

    struct SceneData
    {
        struct Allocator
        {
        };
        Allocator* _dynamicAllocator;
        Allocator* _staticAllocator;
        std::vector<DrawInstanceData> m_data;
        renderer::RenderObjectTracker m_globalResources;

        ViewportState m_viewportState;
        EditorState m_editorState;
        RenderState m_renderState;
    };

} //namespace scene
} //namespace v3d