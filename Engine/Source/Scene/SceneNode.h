#pragma once

#include "Common.h"
#include "Utils/ResourceID.h"
#include "Utils/Copiable.h"

#include "RenderTechniques/RenderPipelinePass.h"

#include "Scene/Transform.h"
#include "Scene/Component.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Component;
    class Model;
    class ModelHandler;
    class SceneHandler;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class TransformMode
    {
        Local,
        Global
    };

    /**
    * @brief SceneNode class
    */
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

        void setVisible(bool visible);
        void setDebug(bool debug);

        math::Vector3D getDirection() const;
        const Transform& getTransform() const;
        const Transform& getPrevTransform() const;
        const Transform& getTransform(TransformMode mode) const;
        bool isVisible() const;

    public:

        SceneNode*                              m_parent;
        std::list<SceneNode*>                   m_children;
        std::list<std::tuple<Component*, bool>> m_components;
        std::string                             m_name;

    private:

        //Instance state
        Transform             m_transform[2];
        Transform             m_prevTransform;

        bool                  m_visible = true;
        bool                  m_debug = false;
        bool                  m_dirty = true;

    protected:

        SceneNode(const SceneNode& node) noexcept;
        virtual ~SceneNode();

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);
        friend Model;
        friend ModelHandler;
        friend SceneHandler;
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

    inline void SceneNode::setVisible(bool visible)
    {
        m_visible = visible;
        m_dirty = true;
    }

    inline void SceneNode::setDebug(bool debug)
    {
        m_debug = debug;
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

    inline bool SceneNode::isVisible() const
    {
        return m_visible;
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
        NodeEntry() noexcept;
        virtual ~NodeEntry() = default;

        SceneNode*  object;
        u32         passMask;
        u32         pipelineID;
    };

    struct DrawNodeEntry final : NodeEntry
    {
        DrawNodeEntry() noexcept;

        Component* geometry;
        Component* material;
    };

    struct LightNodeEntry final : NodeEntry
    {
        LightNodeEntry() noexcept;

        Component* light;
    };

    struct SkyboxNodeEntry final  : NodeEntry
    {
        SkyboxNodeEntry() noexcept;

        Component* skybox;
        Component* material;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d