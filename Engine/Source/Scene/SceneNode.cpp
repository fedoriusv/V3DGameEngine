#include "SceneNode.h"
#include "Scene.h"

namespace v3d
{
namespace scene
{

SceneNode::SceneNode() noexcept
    : m_parent(nullptr)
{
}

SceneNode::SceneNode(const SceneNode& node) noexcept
    : m_parent(node.m_parent)
{
}

SceneNode::~SceneNode()
{
    for (auto& component : m_components)
    {
        if (std::get<1>(component))
        {
            V3D_DELETE(std::get<0>(component), memory::MemoryLabel::MemoryObject);
        }
    }
    m_components.clear();

    for (auto& child : m_children)
    {
        V3D_DELETE(child, memory::MemoryLabel::MemoryObject);
    }
    m_children.clear();
}

void SceneNode::addChild(SceneNode* node)
{
    m_children.push_back(node);
    ASSERT(node->m_parent == nullptr, "node has parend already");
    node->m_parent = this;
}

NodeEntry::NodeEntry() noexcept
    : object(nullptr)
    , passMask(1 << toEnumType(scene::ScenePass::Custom))
    , pipelineID(0)
{
}

DrawNodeEntry::DrawNodeEntry() noexcept
    : geometry(nullptr)
    , material(nullptr)
{
}

LightNodeEntry::LightNodeEntry() noexcept
    : light(nullptr)
{
}

SkyboxNodeEntry::SkyboxNodeEntry() noexcept
    : skybox(nullptr)
    , material(nullptr)
{
}

} //namespace scene
} //namespace v3d