#pragma once

#include "Common.h"
#include "Thread/ThreadSafeAllocator.h"
#include "Task/TaskScheduler.h"

#include "RenderTechniques/RenderObjectTracker.h"
#include "RenderTechniques/RenderPipelinePass.h"

#include "Scene/Camera/CameraController.h"
#include "Scene/Light.h"

#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineStage;
    class RenderTechnique;

    class SceneHandler;
    class SceneNode;
    struct NodeEntry;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct alignas(16) ViewportState
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
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxShadowmapCascadeCount = 4;
    constexpr u32 k_maxPunctualShadowmapCount = 6;

    struct Settings
    {
        struct ViewportParams
        {
            f32              _fov = 60.f;
            f32              _near = 0.1f;
            f32              _far = 1000.f;
            f32              _moveSpeed = 1.0f;
            f32              _rotateSpeed = 25.0f;
            renderer::Format _colorFormat = renderer::Format_R16G16B16A16_SFloat;
            renderer::Format _depthFormat = renderer::Format_D24_UNorm_S8_UInt;
            u32              _renderTargetID = 0;
        } _vewportParams;

        struct ShadowsParams
        {
            math::Dimension2D                           _size = { 2048, 2048 };
            std::array<f32, k_maxShadowmapCascadeCount> _cascadeBaseBias = { 0.0, 0.0, 0.0, 0.0 };
            std::array<f32, k_maxShadowmapCascadeCount> _cascadeSlopeBias = { 0.0, 0.0, 0.0, 0.0 };
            u32                                         _cascadeCount = k_maxShadowmapCascadeCount;
            f32                                         _longRange = 50.0f;
            f32                                         _splitFactor = 0.90f;
            f32                                         _punctualLightBias = 0.0f;
            f32                                         _PCF = 1.0f; //0 - disable; 1 - 3x3;
            f32                                         _textelScale = 0.5f;
            bool                                        _debugShadowCascades = false;
            bool                                        _debugPunctualLightShadows = false;
        } _shadowsParams;

        struct TonemapParams
        {
            u32 _tonemapper = 0;
            u32 _lut = 0;
            f32 _gamma = 2.2f;
            f32 _ev100 = 1.0;
        } _tonemapParams;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief FrameData class
    */
    struct FrameData
    {
        scene::RenderObjectTracker   m_frameResources;
        thread::ThreadSafeAllocator* m_allocator;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SceneData class
    */
    class SceneData
    {
    public:

        SceneData() noexcept;
        virtual ~SceneData();

        scene::FrameData& sceneFrameData() const;
        scene::FrameData& renderFrameData() const;
        u32 numberOfFrames() const;

        const std::vector<SceneNode*>& getNodeList() const;

    public:

        mutable scene::RenderObjectTracker      m_globalResources;
        Settings                                m_settings;

        std::vector<NodeEntry*>                 m_generalRenderList;
        std::vector<NodeEntry*>                 m_renderLists[toEnumType(RenderPipelinePass::Count)];


        math::Dimension2D                       m_viewportSize;
        scene::CameraController*                m_camera;

    protected:

        mutable task::TaskScheduler             m_taskWorker;

        std::vector<SceneNode*>                 m_nodes;

        mutable std::array<scene::FrameData, 1> m_frameState;
        u32                                     m_stateIndex;

        void finalize();

        friend RenderPipelineStage;
        friend SceneHandler;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SceneHandler class
    */
    class SceneHandler
    {
    public:

        explicit SceneHandler(bool isEditor) noexcept;
        virtual ~SceneHandler();

        bool isEditorMode() const;

    protected:

        void create(renderer::Device* device);
        void destroy(renderer::Device* device);

        void updateScene(f32 dt);
        void preRender(renderer::Device* device, f32 dt);
        void postRender(renderer::Device* device, f32 dt);
        void submitRender(renderer::Device* device);

        void addNode(SceneNode* node);
        /*void removeNode(SceneNode* node);
        void updateNode(SceneNode* node)*/

        void registerTechnique(scene::RenderTechnique* technique);
        void unregisterTechnique(scene::RenderTechnique* technique);

        SceneData m_sceneData;

    private:

        std::vector<scene::RenderTechnique*> m_renderTechniques;

        bool                                 m_editorMode;
        bool                                 m_nodeGraphChanged;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::ViewportState>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d