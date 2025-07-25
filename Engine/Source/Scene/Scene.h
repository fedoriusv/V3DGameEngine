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
    enum class MaterialType
    {
        Opaque,
        SkinnedOpaque,
        MaskedOpaque,
        
        Transparency,
        SkinnedTransparency,

        Lights,
        Shadowmap,
        Selected,
        Debug,
        Custom,

        Count
    };

    struct ViewportState
    {
        alignas(16) struct ViewportBuffer
        {
            math::Matrix4D projectionMatrix;
            math::Matrix4D invProjectionMatrix;
            math::Matrix4D viewMatrix;
            math::Matrix4D invViewMatrix;
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

    struct GeomtryState
    {
        utils::StringID         _ID;
        renderer::IndexBuffer*  _idxBuffer;
        renderer::VertexBuffer* _vtxBuffer;
    };

    struct MaterialState
    {
        MaterialType            _type;
        renderer::SamplerState* _sampler;
        renderer::Texture2D*    _albedo;
        renderer::Texture2D*    _normals;
        renderer::Texture2D*    _material;
        math::float4            _tint;
    };

    struct DirectionalLightState
    {
        scene::Transform    _transform;
        math::float4        _color;
        f32                 _attenuation;
        f32                 _intensity;
        f32                 _temperature;
    };

    struct DrawInstanceDataState
    {
        DrawInstanceDataState* _parent = nullptr;
        GeomtryState           _geometry;
        MaterialState          _material;

        scene::Transform       _transform;
        scene::Transform       _prevTransform;

        u64                    _pipelineID;
        u64                    _objectID;

        bool                   _visible = true;
        bool                   _selected = false;
    };

    struct RenderState
    {
        renderer::CmdListRender* m_cmdList;
    };

    struct SceneData
    {
        renderer::RenderObjectTracker              m_globalResources;

        std::vector<scene::DrawInstanceDataState*> m_generalList;
        std::vector<scene::DrawInstanceDataState*> m_lists[toEnumType(MaterialType::Count)];

        ViewportState                              m_viewportState;
        RenderState                                m_renderState;
        DirectionalLightState                      m_diectionalLightState;
    };

    struct FrameData
    {
        struct Allocator
        {
        };
        Allocator* _dynamicAllocator;
        Allocator* _staticAllocator;
    };

} //namespace scene
} //namespace v3d