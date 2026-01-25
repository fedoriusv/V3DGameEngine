#include "Scene.h"

#include "Geometry/Mesh.h"
#include "Material.h"
#include "Billboard.h"
#include "Skybox.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Scene::Scene() noexcept
    : m_frameCounter(0)
    , m_editorMode(false)
{

    m_frameState.resize(3, {});
    m_stateIndex = 0;
}

Scene::~Scene()
{
}

void Scene::beginFrame()
{
}

void Scene::endFrame()
{
    ++m_frameCounter;
}

void Scene::finalize()
{
    static std::function<void(scene::SceneNode* node)> processNode = [&](scene::SceneNode* node)
        {
            if (scene::Mesh* geometry = node->getComponentByType<scene::Mesh>(); geometry)
            {
                scene::Material* material = node->getComponentByType<scene::Material>();

                scene::DrawNodeEntry* entry = new scene::DrawNodeEntry;
                entry->object = node;
                entry->geometry = geometry;
                entry->material = material;
                if (material)
                {
                    if (material->getShadingModel() == scene::MaterialShadingModel::Custom)
                    {
                        entry->passMask = 1 << toEnumType((scene::RenderPipelinePass)material->getProperty<u32>("materialID"));
                        entry->pipelineID = material->getProperty<u32>("pipelineID");
                    }
                    else if (material->getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness || material->getShadingModel() == scene::MaterialShadingModel::PBR_Specular)
                    {
                        bool isOpaque = true;
                        entry->passMask = isOpaque ? (1 << toEnumType(scene::RenderPipelinePass::Opaque)) : (1 << toEnumType(scene::RenderPipelinePass::Transparency));
                        entry->pipelineID = material->getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness ? 0 : 1;
                    }
                }

                if (geometry->isShadowsCasted())
                {
                    entry->passMask |= 1 << toEnumType(scene::RenderPipelinePass::Shadowmap);
                }

                m_sceneData.m_generalRenderList.push_back(entry);
            }

            if (scene::Billboard* unlit = node->getComponentByType<scene::Billboard>(); unlit)
            {
                scene::DrawNodeEntry* entry = new scene::DrawNodeEntry;
                entry->object = node;
                entry->material = node->getComponentByType<scene::Material>();
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::Indicator);
                entry->pipelineID = 0;

                m_sceneData.m_generalRenderList.push_back(entry);
            }

            if (scene::DirectionalLight* light = node->getComponentByType<scene::DirectionalLight>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::DirectionLight);
                entry->pipelineID = 0;

                m_sceneData.m_generalRenderList.push_back(entry);
            }
            else if (scene::Light* light = node->getComponentByType<scene::Light>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::PunctualLights);

                m_sceneData.m_generalRenderList.push_back(entry);
            }
            else if (scene::Skybox* skybox = node->getComponentByType<scene::Skybox>(); skybox)
            {
                scene::Material* material = node->getComponentByType<scene::Material>();

                scene::SkyboxNodeEntry* entry = new scene::SkyboxNodeEntry;
                entry->object = node;
                entry->material = material;
                entry->skybox = skybox;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::Skybox);
                entry->pipelineID = material->getProperty<u32>("pipelineID");

                m_sceneData.m_generalRenderList.push_back(entry);
            }

            for (auto& child : node->m_children)
            {
                processNode(child);
            }
        };

    for (auto& node : m_sceneData.m_nodes)
    {
        processNode(node);
    }
}

} //namespace scene
} //namespace v3d