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
}

void SceneNode::addChild(SceneNode* node)
{
    m_children.push_back(node);
    ASSERT(node->m_parent == nullptr, "node has parend already");
    node->m_parent = this;
}

} //namespace scene
} //namespace v3d