#pragma once

#include "Common.h"
#include "Utils/ResourceID.h"
#include "Utils/Copiable.h"
#include "Task/TaskScheduler.h"
#include "Scene/Transform.h"
#include "Scene/Camera/CameraController.h"
#include "Scene/Light.h"

#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"

#include "RenderTechniques/RenderPipelinePass.h"
#include "RenderTechniques/RenderObjectTracker.h"

#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Component;
    class Model;
    class ModelHandler;

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

        } _viewportBuffer;

        math::Dimension2D        _viewpotSize;
        scene::CameraController* _camera;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxShadowmapCascadeCount = 4;
    constexpr u32 k_maxPunctualShadowmapCount = 8;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Settings
    {
        struct ViewportParams
        {
            f32 _fov = 60.f;
            f32 _near = 0.1f;
            f32 _far = 1000.f;
            f32 _moveSpeed = 1.0f;
            f32 _rotateSpeed = 25.0f;
        } _vewportParams;

        struct ShadowsParams
        {
            math::Dimension2D                           _size = { 2048, 2048 };
            std::array<f32, k_maxShadowmapCascadeCount> _cascadeBaseBias = { 0.0, 0.0, 0.0, 0.0 };
            std::array<f32, k_maxShadowmapCascadeCount> _cascadeSlopeBias = { 0.0, 0.0, 0.0, 0.0 };
            u32                                         _cascadeCount = k_maxShadowmapCascadeCount;
            f32                                         _longRange = 50.0f;
            f32                                         _splitFactor = 0.90f;
            bool                                        _debug = false;
        } _shadowsParams;

        struct TonemapParams
        {
            u32 _tonemapper = 0;
            u32 _lut = 0;
            f32 _gamma = 2.2f;
            f32 _ev100 = 1.0;
        } _tonemapParams;

        renderer::Format _colorFormat = renderer::Format_R16G16B16A16_SFloat;
        renderer::Format _depthFormat = renderer::Format_D24_UNorm_S8_UInt;
    };

    enum class TransformMode
    {
        Local,
        Global
    };

    class SceneNode : public utils::ResourceID<SceneNode, u64>
    {
    public:

        SceneNode() noexcept;

        void addChild(SceneNode* node);
        void addComponent(Component* component, bool owner = true);

        template<class TComponent>
        TComponent* getComponentByType()
        {
            for (auto& [component, owner] : m_components)
            {
                if (component->isBaseOfType<TComponent>())
                {
                    return static_cast<TComponent*>(component);
                }
            }

            return nullptr;
        }

        static void forEach(SceneNode* node, const std::function<void(SceneNode* parent, SceneNode* node)>& entry);
        static SceneNode* searchNode(SceneNode* node, const std::function<bool(SceneNode* node)>& entry);

    public:

        void setPosition(TransformMode mode, const math::Vector3D& position);
        void setRotation(TransformMode mode, const math::Vector3D& rotation);
        void setScale(TransformMode mode, const math::Vector3D& scale);
        void setTransform(TransformMode mode, const math::Matrix4D& transform);

        math::Vector3D getDirection() const;
        const Transform& getTransform() const;
        const Transform& getPrevTransform() const;
        const Transform& getTransform(TransformMode mode) const;

    public:

        SceneNode*                              m_parent;
        std::list<SceneNode*>                   m_children;
        std::list<std::tuple<Component*, bool>> m_components;
        std::string                             m_name;
        bool                                    m_visible = true;

    private:

        //Instance state
        Transform             m_transform[2];
        Transform             m_prevTransform;

        bool                  m_dirty = true;

    protected:

        SceneNode(const SceneNode& node) noexcept;
        virtual ~SceneNode();

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);
        friend Model;
        friend ModelHandler;
    };

    inline void SceneNode::addComponent(Component* component, bool owner)
    {
        m_components.emplace_back(component, owner);
    }

    inline void SceneNode::setPosition(TransformMode mode, const math::Vector3D& position)
    {
        m_transform[toEnumType(mode)].setPosition(position);
        m_dirty = true;
    }

    inline void SceneNode::setRotation(TransformMode mode, const math::Vector3D& rotation)
    {
        m_transform[toEnumType(mode)].setRotation(rotation);
        m_dirty = true;
    }

    inline void SceneNode::setScale(TransformMode mode, const math::Vector3D& scale)
    {
        m_transform[toEnumType(mode)].setScale(scale);
        m_dirty = true;
    }

    inline void SceneNode::setTransform(TransformMode mode, const math::Matrix4D& transform)
    {
        m_transform[toEnumType(mode)].setMatrix(transform);
        m_dirty = true;
    }

    inline math::Vector3D SceneNode::getDirection() const
    {
        return math::Vector3D(
            m_transform[toEnumType(TransformMode::Global)].getMatrix()[8],
            m_transform[toEnumType(TransformMode::Global)].getMatrix()[9],
            m_transform[toEnumType(TransformMode::Global)].getMatrix()[10]).normalize();
    }

    inline const Transform& SceneNode::getTransform() const
    {
        return m_transform[toEnumType(TransformMode::Global)];
    }

    inline const Transform& SceneNode::getPrevTransform() const
    {
        return m_prevTransform;
    }

    inline const Transform& SceneNode::getTransform(TransformMode mode) const
    {
        return m_transform[toEnumType(mode)];
    }

    inline void SceneNode::forEach(SceneNode* node, const std::function<void(SceneNode* parent, SceneNode* node)>& entry)
    {
        std::invoke(entry, node->m_parent, node);

        for (SceneNode* childNode : node->m_children)
        {
            forEach(childNode, entry);
        }
    }

    inline SceneNode* SceneNode::searchNode(SceneNode* node, const std::function<bool(SceneNode* node)>& entry)
    {
        if (entry(node))
        {
            return node;
        }

        for (SceneNode* childNode : node->m_children)
        {
            return searchNode(childNode, entry);
        }

        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct NodeEntry
    {
        SceneNode* object = nullptr;
        u32 passMask = 1 << toEnumType(scene::RenderPipelinePass::Custom);
        u32 pipelineID = 0;
    };

    struct DrawNodeEntry : NodeEntry
    {
        Component* geometry = nullptr;
        Component* material = nullptr;
    };

    struct LightNodeEntry : NodeEntry
    {
        Component* light = nullptr;
    };

    struct SkyboxNodeEntry : NodeEntry
    {
        Component* skybox = nullptr;
        Component* material = nullptr;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct SceneData
    {
        scene::RenderObjectTracker m_globalResources;
        std::vector<SceneNode*>    m_nodes;
        std::vector<NodeEntry*>    m_generalRenderList;
        std::vector<NodeEntry*>    m_renderLists[toEnumType(RenderPipelinePass::Count)];

        ViewportState              m_viewportState;
        Settings                   m_settings;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct FrameData
    {
        struct Allocator
        {
        };

        Allocator* _dynamicAllocator;
        Allocator* _staticAllocator;

        renderer::CmdListRender* m_cmdList;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Scene
    {
    public:
        Scene() noexcept;
        virtual ~Scene();

        virtual void create(renderer::Device* device, const math::Dimension2D& viewportSize) = 0;
        virtual void destroy() = 0;

        virtual void beginFrame();
        virtual void endFrame();

        virtual void preRender(f32 dt) = 0;
        virtual void postRender(f32 dt) = 0;

        virtual void submitRender() = 0;

        void finalize();

    public:

        scene::SceneData                m_sceneData;
        std::vector<scene::FrameData>   m_frameState;
        u32                             m_stateIndex;

        u64                             m_frameCounter;
        bool                            m_editorMode;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d