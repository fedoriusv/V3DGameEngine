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
            if (!node->isVisible())
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
                entry->passMask = 0;
                if (material)
                {
                    if (material->getShadingModel() == scene::MaterialShadingModel::Custom)
                    {
                        entry->passMask |= 1 << toEnumType((scene::ScenePass)material->getProperty<u32>("materialID"));
                        entry->pipelineID = material->getProperty<u32>("pipelineID");
                    }
                    else if (material->getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness)
                    {
                        bool isOpaque = true;
                        entry->passMask |= isOpaque ? (1 << toEnumType(scene::ScenePass::Opaque)) : (1 << toEnumType(scene::ScenePass::Transparency));
                        entry->pipelineID = material->getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness ? 0 : 1;
                    }
                }

                if (geometry->isCastShadow())
                {
                    entry->passMask |= 1 << toEnumType(scene::ScenePass::Shadowmap);
                }

                m_generalRenderList.push_back(entry);
            }

            if (scene::Billboard* unlit = node->getComponentByType<scene::Billboard>(); unlit)
            {
                scene::DrawNodeEntry* entry = new scene::DrawNodeEntry;
                entry->object = node;
                entry->material = node->getComponentByType<scene::Material>();
                entry->geometry = nullptr;
                entry->passMask = 1 << toEnumType(scene::ScenePass::Indicator);
                entry->pipelineID = 0;

                m_generalRenderList.push_back(entry);
            }

            if (scene::DirectionalLight* light = node->getComponentByType<scene::DirectionalLight>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::ScenePass::DirectionLight);
                entry->pipelineID = 0;

                m_generalRenderList.push_back(entry);
            }
            else if (scene::Light* light = node->getComponentByType<scene::Light>(); light)
            {
                scene::LightNodeEntry* entry = new scene::LightNodeEntry;
                entry->object = node;
                entry->light = light;
                entry->passMask = 1 << toEnumType(scene::ScenePass::PunctualLights);

                m_generalRenderList.push_back(entry);
            }
            else if (scene::Skybox* skybox = node->getComponentByType<scene::Skybox>(); skybox)
            {
                scene::Material* material = node->getComponentByType<scene::Material>();

                scene::SkyboxNodeEntry* entry = new scene::SkyboxNodeEntry;
                entry->object = node;
                entry->material = material;
                entry->skybox = skybox;
                entry->passMask = 1 << toEnumType(scene::ScenePass::Skybox);
                entry->pipelineID = material->getProperty<u32>("pipelineID");

                m_generalRenderList.push_back(entry);
            }

            for (auto& child : node->m_children)
            {
                processNode(child);
            }
        };

    //todo use custom alloc for objects
    for (auto& entry : m_generalRenderList)
    {
        delete entry;
    }
    m_generalRenderList.clear(); 

    for (auto& node : m_nodes)
    {
        processNode(node);
    }
}


SceneHandler::SceneHandler(bool isEditor) noexcept
    : m_nodeGraphChanged(true)
{
    m_sceneData.m_editorMode = isEditor;
}

SceneHandler::~SceneHandler()
{
}

bool SceneHandler::isEditorMode() const
{
    return m_sceneData.m_editorMode;
}

void SceneHandler::setupRender(renderer::Device* device)
{
    m_device = device;
}

void SceneHandler::create()
{
    for (auto& frame : m_sceneData.m_frameState)
    {
        frame.m_allocator = V3D_NEW(thread::ThreadSafeAllocator, memory::MemoryLabel::MemoryObject)(4 * 1024 * 1024, m_sceneData.m_taskWorker.getNumberOfCoreThreads());
    }

    for (auto& technique : m_renderTechniques)
    {
        technique->create(m_device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::destroy()
{
    for (auto& technique : m_renderTechniques)
    {
        technique->destroy(m_device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }

    for (auto& frame : m_sceneData.m_frameState)
    {
        V3D_DELETE(frame.m_allocator, memory::MemoryLabel::MemoryObject);
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

    for (u32 i = 0; i < toEnumType(ScenePass::Count); ++i)
    {
        m_sceneData.m_renderLists[toEnumType(ScenePass(i))].clear();
    }

    static auto applyTransfrom = [](SceneNode* parent, SceneNode* node)
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
            SceneNode::forEach(item->object, applyTransfrom);
            item->object->m_dirty = false;
        }

        //fructum test
        //TODO

        u32 count = std::bit_width(item->passMask);
        for (u32 index = 0; index < count; ++index)
        {
            u32 passID = 1 << index;
            if ((item->passMask & passID) == 0)
            {
                continue;
            }

            if (passID & (1 << toEnumType(ScenePass::Shadowmap)))
            {
                //Skip geometry if an object far away from long range distance
                f32 distance = item->object->getTransform().getPosition().distanceFrom(m_sceneData.m_camera->getPosition());
                if (distance > m_sceneData.m_settings._shadowsParams._longRange)
                {
                    continue;
                }
            }

            ASSERT(index < toEnumType(ScenePass::Count), "out of range");
            m_sceneData.m_renderLists[index].push_back(item);
        }
    }

    auto& lightList = m_sceneData.m_renderLists[toEnumType(ScenePass::PunctualLights)];
    std::sort(lightList.begin(), lightList.end(), [camera = m_sceneData.m_camera](const NodeEntry* a, const NodeEntry* b) -> bool
        {
            f32 dist0 = a->object->getTransform().getPosition().distanceFrom(camera->getPosition());
            f32 dist1 = a->object->getTransform().getPosition().distanceFrom(camera->getPosition());
            
            //TODO sort by radius
            return dist0 < dist1;
        });

    for (u32 i = 0; i < std::min<u32>(lightList.size(), k_maxPunctualShadowmapCount); ++i)
    {
        for (auto& item : m_sceneData.m_renderLists[toEnumType(ScenePass::Shadowmap)])
        {
            const NodeEntry* light = lightList[i];
            f32 distance = item->object->getTransform().getPosition().distanceFrom(light->object->getTransform().getPosition());

            if (PointLight* point = light->object->getComponentByType<PointLight>(); point)
            {
                if (point->getRadius() > distance)
                {
                    u32 lightList = toEnumType(scene::ScenePass::FirstPunctualShadowmap) + i;
                    item->passMask |= 1 << lightList;
                    m_sceneData.m_renderLists[lightList].push_back(item);
                }
            }
        }
    }
}

void SceneHandler::preRender(f32 dt)
{
    for (auto& technique : m_renderTechniques)
    {
        technique->prepare(m_device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::postRender(f32 dt)
{
    for (auto& technique : m_renderTechniques)
    {
        technique->execute(m_device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }
}

void SceneHandler::submitRender()
{
    m_sceneData.m_taskWorker.mainThreadLoop();

    for (auto& technique : m_renderTechniques)
    {
        technique->submit(m_device, m_sceneData, m_sceneData.m_frameState[m_sceneData.m_stateIndex]);
    }

    m_sceneData.m_stateIndex = (m_sceneData.m_stateIndex + 1) % m_sceneData.m_frameState.size();
}

void SceneHandler::addNode(SceneNode* node)
{
    m_sceneData.m_nodes.push_back(node);
    nodeGraphChanged();
}

void SceneHandler::nodeGraphChanged()
{
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

DirectionalLight* SceneHandler::addDirectionLightComponent(const math::Vector3D& direction, const color::ColorRGBAF& color, const std::string& name, SceneNode* parent)
{
    SceneNode* directionallightNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
    directionallightNode->m_name = "DirectionLight";
    directionallightNode->setPosition(TransformMode::Local, { 0.0f, 3.0f, 0.0f });
    directionallightNode->setRotation(TransformMode::Local, direction);
    if (parent)
    {
        parent->addChild(directionallightNode);
    }
    else
    {
        addNode(directionallightNode);
    }

    DirectionalLight* directionalLight = V3D_NEW(DirectionalLight, memory::MemoryLabel::MemoryObject)(m_device);
    directionalLight->m_header.setName(name);
    directionalLight->setColor(color);
    directionalLight->setIntensity(5.f);
    directionalLight->setTemperature(4000.0f);
    directionallightNode->addComponent(directionalLight);

    if (isEditorMode())
    {
        ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
        ASSERT(uv_h.isValid(), "must be valid");
        renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

        Billboard* icon = V3D_NEW(Billboard, memory::MemoryLabel::MemoryObject)(m_device);
        directionallightNode->addComponent(icon);

        Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
        material->setProperty("Color", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        material->setProperty("BaseColor", uvGrid);
        directionallightNode->addComponent(material);

        {
            SceneNode* debugNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
            debugNode->m_name = "LightDebug";
            debugNode->setRotation(TransformMode::Local, { 90.f, 0.0f, 0.0f });
            debugNode->setPosition(TransformMode::Local, { 0.0f, 0.0f, 0.25f });
            directionallightNode->addChild(debugNode);

            Mesh* cylinder = MeshHelper::createCylinder(m_device, 0.01f, 0.5f, 16, "lightDirection");
            cylinder->setCastShadow(false);
            debugNode->addComponent(cylinder);

            Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
            material->setProperty("materialID", toEnumType(ScenePass::Debug));
            material->setProperty("pipelineID", 0U);
            material->setProperty("DiffuseColor", color);
            debugNode->addComponent(material);
        }
    }

    return directionalLight;
}

PointLight* SceneHandler::addPointLightComponent(const math::Vector3D& position, f32 radius, const color::ColorRGBAF& color, const std::string& name, scene::SceneNode* parent)
{
    scene::SceneNode* pointLightNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
    pointLightNode->m_name = name;
    pointLightNode->setPosition(scene::TransformMode::Local, position);
    pointLightNode->setScale(scene::TransformMode::Local, { radius, radius, radius });

    if (parent)
    {
        parent->addChild(pointLightNode);
    }
    else
    {
        addNode(pointLightNode);
    }

    PointLight* pointLight = V3D_NEW(PointLight, memory::MemoryLabel::MemoryObject)(m_device);
    pointLight->m_header.setName(name);
    pointLight->setColor(color);
    pointLight->setIntensity(30.f);
    pointLight->setTemperature(4000.0f);
    pointLight->setAttenuation(1.0, 0.09, 0.032);
    pointLight->setRadius(radius);
    pointLightNode->addComponent(pointLight);

    if (isEditorMode())
    {
        ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
        ASSERT(uv_h.isValid(), "must be valid");
        renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

        scene::Billboard* icon = V3D_NEW(Billboard, memory::MemoryLabel::MemoryObject)(m_device);
        pointLightNode->addComponent(icon);

        scene::Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
        material->setProperty("Color", math::float4{ 1.0f, 1.0f, 1.0f, 1.0f });
        material->setProperty("BaseColor", uvGrid);
        pointLightNode->addComponent(material);

        {
            SceneNode* debugNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
            debugNode->m_name = "LightDebug";
            pointLightNode->addChild(debugNode);

            Mesh* sphere = MeshHelper::createSphere(m_device, 1.f, 8, 8, "pointLight");
            sphere->setCastShadow(false);
            debugNode->addComponent(sphere);

            Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
            material->setProperty("materialID", toEnumType(ScenePass::Debug));
            material->setProperty("pipelineID", 1U);
            material->setProperty("DiffuseColor", color);
            debugNode->addComponent(material);
        }
    }

    return pointLight;
}

SpotLight* SceneHandler::addSpotLightComponent(const math::Vector3D& position, f32 range, f32 apexAngle, const color::ColorRGBAF& color, const std::string& name, scene::SceneNode* parent)
{
    SceneNode* spotLightNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
    spotLightNode->m_name = name;

    f32 scale = range * tanf(apexAngle * math::k_degToRad);
    spotLightNode->setScale(TransformMode::Local, { scale, scale, range });
    spotLightNode->setPosition(TransformMode::Local, position);
    spotLightNode->setRotation(TransformMode::Local, { 15.f, 0.f, 0.f });

    if (parent)
    {
        parent->addChild(spotLightNode);
    }
    else
    {
        addNode(spotLightNode);
    }

    SpotLight* spotLight = V3D_NEW(SpotLight, memory::MemoryLabel::MemoryObject)(m_device);
    spotLight->m_header.setName(name);
    spotLight->setRange(range);
    spotLight->setOuterAngle(apexAngle);
    spotLight->setColor(color);
    spotLight->setIntensity(30.f);
    spotLight->setTemperature(4000.0f);
    spotLight->setAttenuation(1.0, 0.09, 0.032);
    spotLightNode->addComponent(spotLight);

    if (isEditorMode())
    {
        ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
        ASSERT(uv_h.isValid(), "must be valid");
        renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

        Billboard* icon = V3D_NEW(Billboard, memory::MemoryLabel::MemoryObject)(m_device);
        spotLightNode->addComponent(icon);

        Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
        material->setProperty("Color", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        material->setProperty("BaseColor", uvGrid);
        spotLightNode->addComponent(material);

        {
            SceneNode* debugNode = V3D_NEW(SceneNode, memory::MemoryLabel::MemoryGame);
            debugNode->m_name = "LightDebug";
            spotLightNode->addChild(debugNode);

            Mesh* sphere = MeshHelper::createCone(m_device, 1.f, 1.f, 16, "spotLight");
            sphere->setCastShadow(false);
            debugNode->addComponent(sphere);

            Material* material = V3D_NEW(Material, memory::MemoryLabel::MemoryObject)(m_device);
            material->setProperty("materialID", toEnumType(ScenePass::Debug));
            material->setProperty("pipelineID", 1U);
            material->setProperty("DiffuseColor", color);
            debugNode->addComponent(material);
        }
    }

    return spotLight;
}

} //namespace scene
} //namespace v3d