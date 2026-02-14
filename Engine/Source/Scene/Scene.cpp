#include "Scene.h"

#include "SceneNode.h"
#include "RenderTechniques/RenderPipelineStage.h"

#include "Geometry/Mesh.h"
#include "Material.h"
#include "Billboard.h"
#include "Skybox.h"

namespace v3d
{
namespace scene
{

constexpr u32 k_workerThreadCount = 4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneData::SceneData() noexcept
    : m_taskWorker(k_workerThreadCount, task::TaskDispatcher::WorkerThreadPerCore/* | task::TaskDispatcher::AllowToMainThreadStealTasks*/)
    , m_stateIndex(0)
{
}

SceneData::~SceneData()
{
}


scene::FrameData& SceneData::sceneFrameData() const
{
    return m_frameState[m_stateIndex];
}

scene::FrameData& SceneData::renderFrameData() const
{
    u32 prevIndex = (m_stateIndex + m_frameState.size() - 1) % m_frameState.size();
    return m_frameState[prevIndex];
}

u32 SceneData::numberOfFrames() const
{
    return m_frameState.size();
}

const std::vector<SceneNode*>& SceneData::getNodeList() const
{
    return m_nodes;
}

void SceneData::finalize()
{
    static std::function<void(scene::SceneNode* node)> processNode = [&](scene::SceneNode* node)
        {
            if (!node->m_visible)
            {
                return;
            }

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
                    else if (material->getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness)
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

                m_generalRenderList.push_back(entry);
            }

            if (scene::Billboard* unlit = node->getComponentByType<scene::Billboard>(); unlit)
            {
                scene::DrawNodeEntry* entry = new scene::DrawNodeEntry;
                entry->object = node;
                entry->material = node->getComponentByType<scene::Material>();
                entry->geometry = nullptr;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::Indicator);
                entry->pipelineID = 0;

                m_generalRenderList.push_back(entry);
            }

            if (scene::DirectionalLight* light = node->getComponentByType<scene::DirectionalLight>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::DirectionLight);
                entry->pipelineID = 0;

                m_generalRenderList.push_back(entry);
            }
            else if (scene::Light* light = node->getComponentByType<scene::Light>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::RenderPipelinePass::PunctualLights);

                m_generalRenderList.push_back(entry);
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

                m_generalRenderList.push_back(entry);
            }

            for (auto& child : node->m_children)
            {
                processNode(child);
            }
        };

    for (auto& node : m_nodes)
    {
        processNode(node);
    }
}


SceneHandler::SceneHandler(bool isEditor) noexcept
    : m_editorMode(isEditor)
    , m_nodeGraphChanged(true)
{
}

SceneHandler::~SceneHandler()
{
}

bool SceneHandler::isEditorMode() const
{
    return m_editorMode;
}

void SceneHandler::create(renderer::Device* device)
{
    for (auto& frame : m_sceneData.m_frameState)
    {
        frame.m_allocator = new thread::ThreadSafeAllocator(4 * 1024 * 1024, m_sceneData.m_taskWorker.getNumberOfCoreThreads());
    }

    for (auto& technique : m_renderTechniques)
    {
        technique->create(device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::destroy(renderer::Device* device)
{
    for (auto& technique : m_renderTechniques)
    {
        technique->destroy(device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }

    for (auto& frame : m_sceneData.m_frameState)
    {
        delete frame.m_allocator;
        frame.m_allocator = nullptr;
    }
}

void SceneHandler::updateScene(f32 dt)
{
    if (m_nodeGraphChanged)
    {
        m_sceneData.finalize();
        m_nodeGraphChanged = false;
    }

    for (u32 i = 0; i < toEnumType(RenderPipelinePass::Count); ++i)
    {
        m_sceneData.m_renderLists[toEnumType(RenderPipelinePass(i))].clear();
    }

    auto callback = [](SceneNode* parent, SceneNode* node)
        {
            if (parent)
            {
                const math::Matrix4D transform = parent->m_transform[toEnumType(TransformMode::Global)].getMatrix() * node->m_transform[toEnumType(TransformMode::Local)].getMatrix();
                node->m_transform[toEnumType(TransformMode::Global)].setMatrix(transform);
            }
            else
            {
                node->m_transform[toEnumType(TransformMode::Global)].setMatrix(node->m_transform[toEnumType(TransformMode::Local)].getMatrix());
            }
        };

    //group by type
    for (auto& item : m_sceneData.m_generalRenderList)
    {
        if (item->object->m_dirty)
        {
            SceneNode::forEach(item->object, callback);
            item->object->m_dirty = false;
        }

        //fructum test
        //TODO

        if (item->object->m_visible)
        {
            u32 count = std::bit_width(item->passMask);
            for (u32 index = 0; index < count; ++index)
            {
                u32 passID = 1 << index;
                if ((item->passMask & passID) == 0)
                {
                    continue;
                }

                ASSERT(index < toEnumType(RenderPipelinePass::Count), "out of range");
                m_sceneData.m_renderLists[index].push_back(item);
            }
        }
    }
}

void SceneHandler::preRender(renderer::Device* device, f32 dt)
{
    for (auto& technique : m_renderTechniques)
    {
        technique->prepare(device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::postRender(renderer::Device* device, f32 dt)
{
    for (auto& technique : m_renderTechniques)
    {
        technique->execute(device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::submitRender(renderer::Device* device)
{
    m_sceneData.m_taskWorker.mainThreadLoop();

    for (auto& technique : m_renderTechniques)
    {
        technique->submit(device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }

    m_sceneData.m_stateIndex = (m_sceneData.m_stateIndex + 1) % m_sceneData.m_frameState.size();
}

void SceneHandler::addNode(SceneNode* node)
{
    m_sceneData.m_nodes.push_back(node);
    m_nodeGraphChanged = true;
}

void SceneHandler::registerTechnique(scene::RenderTechnique* technique)
{
    m_renderTechniques.push_back(technique);
}

void SceneHandler::unregisterTechnique(scene::RenderTechnique* technique)
{
    //m_renderTechniques.erase(std::destroy(m_renderTechniques.begin(), m_renderTechniques.end(), technique));
}

} //namespace scene
} //namespace v3d