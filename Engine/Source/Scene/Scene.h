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

    class Scene
    {
    public:

        struct Allocator
        {
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


            std::string m_stageID;
            u64 m_pipelineID;
            u64 m_objectID;
        };

        struct ViewportState
        {
            alignas(16) struct ViewportBuffer
            {
                math::Matrix4D projectionMatrix;
                math::Matrix4D viewMatrix;
                math::Matrix4D prevProjectionMatrix;
                math::Matrix4D prevViewMatrix;
                math::Vector4D cameraPosition;
                math::float2   viewportSize;
                math::float2   cursorPosition;
                math::float4   random;
                u64            time;
                math::float2   _unused = {};

            } _viewportBuffer;

            math::Dimension2D m_viewpotSize;
            scene::CameraHandler* m_camera;
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
            Allocator* _dynamicAllocator;
            Allocator* _staticAllocator;
            std::vector<DrawInstanceData> m_data;
            renderer::RenderObjectTracker m_globalResources;

            ViewportState m_viewportState;
            EditorState m_editorState;
            RenderState m_renderState;
        };

        Scene() noexcept = default;
        virtual ~Scene() = default;

    protected:

        std::vector<SceneData> m_states;
        u32 m_stateIndex;
    };

} //namespace scene
} //namespace v3d