#pragma once

#include "Common.h"
#include "Scene/Transform.h"
#include "Scene/Camera/CameraHandler.h"
#include "Scene/Light.h"

#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "RenderTechniques/RenderObjectTracker.h"

#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class MaterialType
    {
        Opaque,
        SkinnedOpaque,
        MaskedOpaque,
        
        Transparency,
        SkinnedTransparency,

        Billboard,
        VFX,

        Lights,
        Shadowmap,

        Selected,
        Debug,

        Custom,

        Count
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //TODO
    struct MaterialState
    {
        renderer::SamplerState* _sampler = nullptr;
        renderer::Texture2D*    _baseColor = nullptr;
        renderer::Texture2D*    _normals = nullptr;
        renderer::Texture2D*    _roughness = nullptr;
        renderer::Texture2D*    _metalness = nullptr;
        math::float4            _tint;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DrawNode
    {
        DrawNode*              _parent = nullptr;
        Renderable*            _object = nullptr;

        MaterialState          _material;
        std::string            _title;
        u64                    _pipelineID;
        u64                    _objectID;
        MaterialType           _type;
        bool                   _visible = true;
        bool                   _selected = false;
    };

    //TODO
    struct LightingState
    {
        DrawNode*         _parent = nullptr;
        DirectionalLight* _directionalLight;
    };

    //TODO remove
    struct RenderState
    {
        renderer::CmdListRender* m_cmdList;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct SceneData
    {
        renderer::RenderObjectTracker   m_globalResources;
                                        
        std::vector<DrawNode*>          m_generalList;
        std::vector<DrawNode*>          m_lists[toEnumType(MaterialType::Count)];
                                        
        ViewportState                   m_viewportState;
        RenderState                     m_renderState;
        LightingState                   m_lightingState;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct FrameData
    {
        struct Allocator
        {
        };
        Allocator* _dynamicAllocator;
        Allocator* _staticAllocator;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d