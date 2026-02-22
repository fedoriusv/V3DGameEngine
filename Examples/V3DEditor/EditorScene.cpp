#include "EditorScene.h"
#include "Utils/Logger.h"

#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"
#include "Resource/Loader/ShaderCompiler.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Geometry/StaticMesh.h"
#include "Scene/Model.h"
#include "Scene/Light.h"
#include "Scene/Billboard.h"
#include "Scene/Skybox.h"
#include "Scene/Material.h"

#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Renderer/ShaderProgram.h"

#include "RenderTechniques/RenderPipelineGBuffer.h"
#include "RenderTechniques/RenderPipelineOutline.h"
#include "RenderTechniques/RenderPipelineTonemapStage.h"
#include "RenderTechniques/RenderPipelineUIOverlay.h"
#include "RenderTechniques/RenderPipelineZPrepass.h"
#include "RenderTechniques/RenderPipelineFXAA.h"
#include "RenderTechniques/RenderPipelineTAA.h"
#include "RenderTechniques/RenderPipelineMBOIT.h"
#include "RenderTechniques/RenderPipelineSelection.h"
#include "RenderTechniques/RenderPipelineDeferredLighting.h"
#include "RenderTechniques/RenderPipelineLightAccumulationStage.h"
#include "RenderTechniques/RenderPipelineDebug.h"
#include "RenderTechniques/RenderPipelineUnlit.h"
#include "RenderTechniques/RenderPipelineSkybox.h"
#include "RenderTechniques/RenderPipelineShadow.h"

#include "Stream/StreamManager.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "FrameProfiler.h"

using namespace v3d;

static auto loadTexture2D = [](const std::string& name, bool generateMips) -> renderer::Texture2D*
    {
        renderer::Texture::LoadPolicy policy;
        policy.usage = renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write;
        policy.generateMipmaps = generateMips;

        return resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>(name, policy);
    };

static auto randomVector = [](f32 min, f32 max) -> math::Vector3D
    {
        f32 x = math::random<f32>(min, max);
        f32 y = math::random<f32>(min, max);
        f32 z = math::random<f32>(min, max);

        return math::Vector3D(x, y, z);
    };


bool EditorScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorScreen::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    return false;
}

EditorScene::RenderPipelineScene::RenderPipelineScene(scene::ModelHandler* modelHandler, ui::WidgetHandler* uiHandler)
{
    new scene::RenderPipelineZPrepassStage(this, modelHandler);
    new scene::RenderPipelineGBufferStage(this, modelHandler);
    new scene::RenderPipelineShadowStage(this, modelHandler);
    new scene::RenderPipelineSelectionStage(this, modelHandler);
    new scene::RenderPipelineDeferredLightingStage(this);
    new scene::RenderPipelineLightAccumulationStage(this, modelHandler);
    new scene::RenderPipelineSkyboxStage(this, modelHandler);
    //new scene::RenderPipelineMBOITStage(this);
    new scene::RenderPipelineUnlitStage(this, modelHandler);
    new scene::RenderPipelineOutlineStage(this);
    new scene::RenderPipelineFXAAStage(this);
    new scene::RenderPipelineTAAStage(this);
    new scene::RenderPipelineDebugStage(this, modelHandler);
    new scene::RenderPipelineTonemapStage(this);
    new scene::RenderPipelineUIOverlayStage(this, uiHandler);
}

EditorScene::RenderPipelineScene::~RenderPipelineScene()
{
}

EditorScene::EditorScene() noexcept
    : SceneHandler(true)
    , m_modelHandler(new scene::ModelHandler())
    , m_UIHandler(nullptr)
    , m_cameraHandler(new scene::CameraEditorHandler(std::make_unique<scene::Camera>()))
    , m_inputHandler(new event::InputEventHandler())
    , m_gameHandler(new event::GameEventHandler())
    , m_gameEventRecevier(new event::GameEventReceiver([](event::GameEvent* event)
        {
            delete event;
        }))

    , m_mainPipeline(m_modelHandler, m_UIHandler)

    , m_hotReload(m_gameEventRecevier)

    , m_frameCounter(0)
    , m_selectedIndex(k_emptyIndex)

{
    m_inputHandler->bind([this](const event::MouseInputEvent* event)
        {
            if (m_currentViewportRect.isPointInside({ (f32)m_inputHandler->getAbsoluteCursorPosition()._x, (f32)m_inputHandler->getAbsoluteCursorPosition()._y }))
            {
                if (event->_event == event::MouseInputEvent::MouseDoubleClick)
                {
                    ObjectHandle selectedObject = m_sceneData.m_globalResources.get("readback_objectIDData");
                    if (selectedObject.isValid())
                    {
                        scene::RenderPipelineOutlineStage::MappedData* readback_objectIDData = objectFromHandle<scene::RenderPipelineOutlineStage::MappedData>(selectedObject);
                        scene::SceneNode* selectedNode = nullptr;
                        m_selectedIndex = k_emptyIndex;
                        if (readback_objectIDData->_ptr)
                        {
                            u32 id = readback_objectIDData->_ptr[0];
                            if (auto found = std::find_if(m_sceneData.m_generalRenderList.cbegin(), m_sceneData.m_generalRenderList.cend(), [id](scene::NodeEntry* node)
                                {
                                    return node->object->ID() == id;
                                }); found != m_sceneData.m_generalRenderList.cend())
                            {
                                selectedNode = (*found)->object;
                                m_selectedIndex = m_sceneData.m_generalRenderList.size() - std::distance(found, m_sceneData.m_generalRenderList.cend());
                            }
                        }
                        m_gameEventRecevier->sendEvent(new EditorSelectionEvent(selectedNode));
                    }
                }
                m_cameraHandler->handleInputEventCallback(m_inputHandler, event);
            }
        }
    );

    m_inputHandler->bind([this](const event::KeyboardInputEvent* event)
        {
            if (m_currentViewportRect.isPointInside({ (f32)m_inputHandler->getAbsoluteCursorPosition()._x, (f32)m_inputHandler->getAbsoluteCursorPosition()._y }))
            {
                if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
                {
                    if (event->_key == event::KeyCode::KeyKey_F) //focus on selected object
                    {
                        m_cameraHandler->setTarget(m_sceneData.m_generalRenderList[m_selectedIndex]->object->getTransform().getPosition());
                    }
                }
                m_cameraHandler->handleInputEventCallback(m_inputHandler, event);
            }
        }
    );

    m_gameHandler->bind([this](const event::GameEvent* event, event::GameEvent::GameEventType type, u64 ID)
        {
            if (event->_eventType == event::GameEvent::GameEventType::Custom && event->_customEventID == toEnumType(EditorEventType::SelectObject))
            {
                const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
                scene::SceneNode* selectedNode = selectionEvent->_node;

                if (auto found = std::find_if(m_sceneData.m_generalRenderList.cbegin(), m_sceneData.m_generalRenderList.cend(), [selectedNode](scene::NodeEntry* node)
                    {
                        return node->object == selectedNode;
                    }); found != m_sceneData.m_generalRenderList.cend())
                {
                    m_selectedIndex = m_sceneData.m_generalRenderList.size() - std::distance(found, m_sceneData.m_generalRenderList.cend());
                }
            }
            if (event->_eventType == event::GameEvent::GameEventType::Custom && event->_customEventID == toEnumType(EditorEventType::UpdateNodeGraph))
            {
                nodeGraphChanged();
            }
            else if (event->_eventType == event::GameEvent::GameEventType::HotReload)
            {
                if (isEditorMode())
                {
                    const event::ShaderHotReload* hotReloadEvent = static_cast<const event::ShaderHotReload*>(event);
                    m_mainPipeline.onChanged(m_device, m_sceneData, hotReloadEvent);
                }
            }
        }
    );

    m_gameEventRecevier->attach(m_gameHandler);
}

EditorScene::~EditorScene()
{
    m_gameEventRecevier->dettach(m_gameHandler);

    delete m_modelHandler;
    //delete m_UIHandler;
    
    delete m_gameEventRecevier;
    delete m_inputHandler;
    delete m_gameHandler;
    delete m_cameraHandler;
}

scene::SceneData& EditorScene::getSceneData()
{
    return m_sceneData;
}

void EditorScene::createScene(renderer::Device* device, const math::Dimension2D& viewportSize)
{
    setupRender(device);

    {
        resource::ResourceManager::createInstance();

        auto textureLoader = std::make_unique<resource::TextureFileLoader>(m_device);
        textureLoader->addRoot("../../../../examples/v3deditor/data/");
        textureLoader->addRoot("../../../../engine/data/");
        textureLoader->addPath("textures/");
        textureLoader->addPath("skybox/");
        textureLoader->addPath("suntemple/");
        resource::ResourceManager::getInstance()->registerLoader<resource::TextureFileLoader::ResourceType>(std::move(textureLoader));

        auto modelLoader = std::make_unique<resource::ModelFileLoader>(m_device);
        modelLoader->addRoot("../../../../examples/v3deditor/data/");
        modelLoader->addRoot("../../../../engine/data/");
        modelLoader->addPath("models/");
        modelLoader->addPath("suntemple/");
        resource::ResourceManager::getInstance()->registerLoader<resource::ModelFileLoader::ResourceType>(std::move(modelLoader));

        auto shaderLoader = std::make_unique<resource::ShaderSourceFileLoader>(m_device, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        shaderLoader->addRoot("../../../../examples/v3deditor/data/");
        shaderLoader->addRoot("../../../../engine/data/");
        shaderLoader->addPath("shaders/");
        resource::ResourceManager::getInstance()->registerLoader<resource::ShaderSourceFileLoader::ResourceType>(std::move(shaderLoader));

        m_hotReload.addFolder("../../../../engine/data/shaders");
    }

    registerTechnique(&m_mainPipeline);

    //load setting from config
    m_sceneData.m_viewportSize = { (u32)viewportSize._width, (u32)viewportSize._height };

    m_cameraHandler->setPerspective(m_sceneData.m_settings._vewportParams._fov, viewportSize, m_sceneData.m_settings._vewportParams._near, m_sceneData.m_settings._vewportParams._far);
    m_cameraHandler->setMoveSpeed(m_sceneData.m_settings._vewportParams._moveSpeed);
    m_cameraHandler->setRotationSpeed(m_sceneData.m_settings._vewportParams._rotateSpeed);
    m_cameraHandler->setTarget({ 0.f, 0.f, 0.f });
    m_cameraHandler->setPosition({ 0.f, 0.25f, -1.f });
    m_currentViewportRect = math::Rect(0, 0, viewportSize._width, viewportSize._height);

    //load default resources
    loadResources();
    if (isEditorMode())
    {
        //editor_loadDebug();
    }

    SceneHandler::create();
}

void EditorScene::destroyScene()
{
    unregisterTechnique(&m_mainPipeline);

    SceneHandler::destroy();
    setupRender(nullptr);
}

void EditorScene::loadScene()
{
    //TODO add open dialog for this
    test_loadTestScene();
    //test_loadScene("SunTemple.fbx");
    /////
}

void EditorScene::beginFrame()
{
}

void EditorScene::endFrame()
{
    ++m_frameCounter;
}

void EditorScene::preRender(f32 dt)
{
    TRACE_PROFILER_SCOPE("PreRender", color::rgba8::WHITE);

    m_gameEventRecevier->sendDeferredEvents();
    m_cameraHandler->update(dt);
    m_sceneData.m_camera = m_cameraHandler;

    s32 posX = (s32)m_inputHandler->getAbsoluteCursorPosition()._x - (s32)m_currentViewportRect.getLeftX();
    posX = (posX < 0) ? 0 : posX;
    s32 posY = (s32)m_inputHandler->getAbsoluteCursorPosition()._y - (s32)m_currentViewportRect.getTopY();
    posY = (posY < 0) ? 0 : posY;

    scene::FrameData& currentFrameData = m_sceneData.sceneFrameData();
    currentFrameData.m_allocator->reset();
    currentFrameData.m_frameResources.cleanup();

    scene::ViewportState* viewportState = currentFrameData.m_allocator->construct<scene::ViewportState>();
    viewportState->prevProjectionMatrix = viewportState->projectionMatrix;
    viewportState->prevViewMatrix = viewportState->viewMatrix;
    viewportState->prevCameraJitter = viewportState->cameraJitter;
    viewportState->projectionMatrix = m_cameraHandler->getCamera().getProjectionMatrix();
    viewportState->invProjectionMatrix = m_cameraHandler->getCamera().getProjectionMatrix().getInversed();
    viewportState->viewMatrix = m_cameraHandler->getCamera().getViewMatrix();
    viewportState->invViewMatrix = m_cameraHandler->getCamera().getViewMatrix().getInversed();
    viewportState->cameraJitter = (m_sceneData.m_settings._vewportParams._antiAliasingMode == scene::AntiAliasing::TAA) ? scene::CameraController::calculateJitter(m_frameCounter, m_sceneData.m_viewportSize) : math::float2{ 0.0f, 0.0f };
    viewportState->cameraPosition = { m_cameraHandler->getPosition().getX(), m_cameraHandler->getPosition().getY(), m_cameraHandler->getPosition().getZ(), 0.f };
    viewportState->viewportSize = { (f32)m_sceneData.m_viewportSize._width, (f32)m_sceneData.m_viewportSize._height };
    viewportState->clipNearFar = { m_cameraHandler->getNear(), m_cameraHandler->getFar() };
    viewportState->random = { math::random<f32>(0.f, 0.1f),math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f) };
    viewportState->cursorPosition = { (f32)posX, (f32)posY };
    viewportState->time = utils::Timer::getCurrentTime();
    currentFrameData.m_frameResources.bind("viewport_state", viewportState);

    if (m_frameCounter <= 0 && m_sceneData.numberOfFrames() > 1) //Need to update data for render frame only for first time
    {
        scene::FrameData& renderFrameData = m_sceneData.renderFrameData();
        renderFrameData.m_allocator->reset();
        renderFrameData.m_frameResources.cleanup();

        scene::ViewportState* renderViewportState = m_sceneData.renderFrameData().m_allocator->construct<scene::ViewportState>();
        memcpy(renderViewportState, viewportState, sizeof(scene::ViewportState));
        renderFrameData.m_frameResources.bind("viewport_state", renderViewportState);
    }

    m_sceneData.m_globalResources.bind("current_lut", std::get<1>(m_LUTs[m_sceneData.m_settings._tonemapParams._lut]));

    SceneHandler::updateScene(dt);
    if (m_selectedIndex != k_emptyIndex && m_sceneData.m_generalRenderList[m_selectedIndex]->object->isVisible())
    {
        m_sceneData.m_renderLists[toEnumType(scene::ScenePass::Selected)].push_back(m_sceneData.m_generalRenderList[m_selectedIndex]);
    }

    m_mainPipeline.getStage("FXAA")->setEnabled(m_sceneData.m_settings._vewportParams._antiAliasingMode == scene::AntiAliasing::FXAA);
    m_mainPipeline.getStage("TAA")->setEnabled(m_sceneData.m_settings._vewportParams._antiAliasingMode == scene::AntiAliasing::TAA);

    SceneHandler::preRender(dt);
}

void EditorScene::postRender(f32 dt)
{
    TRACE_PROFILER_SCOPE("PostRender", color::rgba8::WHITE);

    SceneHandler::postRender(dt);
}

void EditorScene::submitRender()
{
    TRACE_PROFILER_SCOPE("SubmitRender", color::rgba8::WHITE);

    SceneHandler::submitRender();
}

void EditorScene::transformSelectedObject(const math::Matrix4D& transform)
{
    if (m_selectedIndex != k_emptyIndex)
    {
        m_sceneData.m_generalRenderList[m_selectedIndex]->object->setTransform(scene::TransformMode::Local, transform);
    }
}

void EditorScene::onChanged(const v3d::math::Rect& viewport)
{
    if (viewport != m_currentViewportRect)
    {
        if (m_currentViewportRect.getWidth() != viewport.getWidth() || m_currentViewportRect.getHeight() != viewport.getHeight())
        {
            m_sceneData.m_viewportSize = { (u32)viewport.getWidth(), (u32)viewport.getHeight() };
        }

        m_currentViewportRect = viewport;
    }
}

void EditorScene::onChanged(const math::Matrix4D& view)
{
    m_cameraHandler->setViewMatrix(view);
    m_cameraHandler->setTarget(m_cameraHandler->getPosition() + m_cameraHandler->getCamera().getForwardVector() * 2.f);
    m_cameraHandler->update(0.f);
}

renderer::Texture2D* EditorScene::getOutputTexture() const
{
    if (m_sceneData.m_settings._vewportParams._renderTargetID)
    {
        ObjectHandle debug = m_sceneData.m_globalResources.get("final_visualize");
        ASSERT(debug.isValid(), "must be valid");
        renderer::Texture2D* renderTarget = objectFromHandle<renderer::Texture2D>(debug);
        return renderTarget;
    }

    ObjectHandle final = m_sceneData.m_globalResources.get("final_target");
    ASSERT(final.isValid(), "must be valid");
    renderer::Texture2D* renderTarget = objectFromHandle<renderer::Texture2D>(final);
    return renderTarget;
}

const math::Rect& EditorScene::getViewportArea() const
{
    return m_currentViewportRect;
}

scene::Camera* EditorScene::getCamera()
{
    return &m_cameraHandler->getCamera();
}

event::InputEventHandler* EditorScene::getInputHandler()
{
    return m_inputHandler;
}

event::GameEventHandler* EditorScene::getGameHandler()
{
    return m_gameHandler;
}

event::GameEventReceiver* EditorScene::getGameEventReceiver()
{
    return m_gameEventRecevier;
}


void EditorScene::loadResources()
{
    renderer::Texture::LoadPolicy policy;
    policy.usage = renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write;
    policy.generateMipmaps = false;

    renderer::Texture2D* default_black = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_black.dds", policy);
    renderer::Texture2D* default_white = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_white.dds", policy);
    renderer::Texture2D* default_normal = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_normal.dds", policy);
    renderer::Texture2D* default_material = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_material.dds", policy);
    renderer::Texture2D* default_roughness = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_black.dds", policy);
    renderer::Texture2D* default_metalness = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("default_black.dds", policy);
    renderer::Texture2D* uv_grid = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("uv_grid.dds", policy);
    renderer::Texture2D* noise_blue = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("noise_blue.dds", policy);
    renderer::Texture2D* tiling_noise = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>("good64x64tilingnoisehighfreq.dds", policy);
    renderer::Texture3D* default_lut = resource::ResourceManager::getInstance()->load<renderer::Texture3D, resource::TextureFileLoader>("default_lut.dds", policy);
    renderer::Texture3D* greyscale_lut = resource::ResourceManager::getInstance()->load<renderer::Texture3D, resource::TextureFileLoader>("greyscale_lut.dds", policy);

    m_sceneData.m_globalResources.bind("default_black", default_black);
    m_sceneData.m_globalResources.bind("default_white", default_white);
    m_sceneData.m_globalResources.bind("default_normal", default_normal);
    m_sceneData.m_globalResources.bind("default_material", default_material);
    m_sceneData.m_globalResources.bind("default_roughness", default_roughness);
    m_sceneData.m_globalResources.bind("default_metalness", default_metalness);
    m_sceneData.m_globalResources.bind("uv_grid", uv_grid);
    m_sceneData.m_globalResources.bind("noise_blue", noise_blue);
    m_sceneData.m_globalResources.bind("tiling_noise", tiling_noise);
    m_sceneData.m_globalResources.bind("default_lut", default_lut);
    m_sceneData.m_globalResources.bind("greyscale_lut", greyscale_lut);

    renderer::SamplerState* linear_sampler_repeat = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_repeat->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    renderer::SamplerState* linear_sampler_mirror = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_mirror->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    renderer::SamplerState* linear_sampler_clamp_border = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_clamp_border->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    renderer::SamplerState* linear_sampler_clamp_edge = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_clamp_edge->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);

    renderer::SamplerState* nearest_sampler_repeat = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    nearest_sampler_repeat->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    renderer::SamplerState* nearest_sampler_clamp_border = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    nearest_sampler_clamp_border->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    renderer::SamplerState* nearest_sampler_clamp_edge = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    nearest_sampler_clamp_edge->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);

    m_sceneData.m_globalResources.bind("linear_sampler_repeat", linear_sampler_repeat);
    m_sceneData.m_globalResources.bind("linear_sampler_mirror", linear_sampler_mirror);
    m_sceneData.m_globalResources.bind("linear_sampler_clamp_border", linear_sampler_clamp_border);
    m_sceneData.m_globalResources.bind("linear_sampler_clamp_edge", linear_sampler_clamp_edge);
    m_sceneData.m_globalResources.bind("point_sampler_repeat", nearest_sampler_repeat);
    m_sceneData.m_globalResources.bind("point_sampler_clamp_border", nearest_sampler_clamp_border);
    m_sceneData.m_globalResources.bind("point_sampler_clamp_edge", nearest_sampler_clamp_edge);

    m_LUTs.emplace_back("No LUT", default_lut);
    m_LUTs.emplace_back("Default LUT", default_lut);
    m_LUTs.emplace_back("Greyscale LUT", greyscale_lut);
}

void EditorScene::test_loadScene(const std::string& name)
{
    //resource::ResourceManager::getInstance()->addPath("../../../../examples/v3deditor/data/SunTemple/");

    //Config scene
    m_sceneData.m_settings._shadowsParams._longRange = 250.f;
    m_sceneData.m_settings._shadowsParams._cascadeBaseBias = { 0.1f, 0.12f, 0.15f, 0.5f };
    m_sceneData.m_settings._shadowsParams._cascadeSlopeBias = { 1.0f, 1.5f, 2.0f, 3.0f };

    scene::Model::LoadPolicy policy;
    policy.scaleFactor = 0.01f;
    policy.overridedShadingModel = scene::MaterialShadingModel::Custom;

    scene::Model* scene = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>(name, policy, resource::ModelFileLoader::Optimization | resource::ModelFileLoader::OverridedShadingModel | 0);
    ASSERT(scene, "nullptr");

    scene::SceneNode::forEach(scene, [this](scene::SceneNode* parent, scene::SceneNode* node)
        {
            if (scene::Material* material = node->getComponentByType<scene::Material>(); material)
            {
                material->setProperty("pipelineID", 1U);
            }

            if (scene::DirectionalLight* dirLight = node->getComponentByType<scene::DirectionalLight>(); dirLight && isEditorMode())
            {
                node->setScale(scene::TransformMode::Local, { 1.0f, 1.0f, 1.0f });

                ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
                ASSERT(uv_h.isValid(), "must be valid");
                renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

                scene::Billboard* icon = new scene::Billboard(m_device);
                node->addComponent(icon);

                scene::Material* material = new scene::Material(m_device);
                material->setProperty("Color", math::float4{ 1.0, 1.0, 1.0, 1.0 });
                material->setProperty("BaseColor", uvGrid);
                node->addComponent(material);

                {
                    scene::SceneNode* debugNode = new scene::SceneNode();
                    debugNode->m_name = "LightDebug";
                    debugNode->setRotation(scene::TransformMode::Local, { 90.f, 0.0f, 0.0f });
                    debugNode->setPosition(scene::TransformMode::Local, { 0.0f, 0.0f, 0.25f });
                    node->addChild(debugNode);

                    scene::Mesh* cylinder = scene::MeshHelper::createCylinder(m_device, 0.01f, 0.5f, 16, "lightDirection");
                    cylinder->setCastShadow(false);
                    debugNode->addComponent(cylinder);

                    scene::Material* material = new scene::Material(m_device);
                    material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
                    material->setProperty("pipelineID", 0U);
                    material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 0.0, 1.0 });
                    debugNode->addComponent(material);
                }
            }
            else if (scene::PointLight* pointLight = node->getComponentByType<scene::PointLight>(); pointLight && isEditorMode())
            {
                node->setScale(scene::TransformMode::Local, { 10.0f, 10.0f, 10.0f });
                pointLight->setRadius(10.0f);
                pointLight->setAttenuation(1.0, 0.5f, 2.0f);

                ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
                ASSERT(uv_h.isValid(), "must be valid");
                renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

                scene::Billboard* icon = new scene::Billboard(m_device);
                node->addComponent(icon);

                scene::Material* material = new scene::Material(m_device);
                material->setProperty("Color", math::float4{ 1.0, 1.0, 1.0, 1.0 });
                material->setProperty("BaseColor", uvGrid);
                node->addComponent(material);

                {
                    scene::SceneNode* debugNode = new scene::SceneNode();
                    debugNode->m_name = "LightDebug";
                    node->addChild(debugNode);

                    scene::Mesh* sphere = scene::MeshHelper::createSphere(m_device, 1.f, 8, 8, "pointLight");
                    sphere->setCastShadow(false);
                    debugNode->addComponent(sphere);

                    scene::Material* material = new scene::Material(m_device);
                    material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
                    material->setProperty("pipelineID", 1U);
                    material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 0.0, 1.0 });
                    debugNode->addComponent(material);
                }
            }
        });

    addNode(scene);

    {
        scene::SceneNode* skyboxNode = new scene::SceneNode();
        skyboxNode->m_name = "Skybox";
        addNode(skyboxNode);

        scene::Skybox* skybox = new scene::Skybox(m_device);
        skyboxNode->addComponent(skybox);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("BaseColor", loadTexture2D("SunTemple_Skybox.dds", false));
        material->setProperty("pipelineID", 0U);
        skyboxNode->addComponent(material);
    }
}

void EditorScene::test_loadTestScene()
{
    //Config scene
    m_sceneData.m_settings._shadowsParams._longRange = 50.f;
    m_sceneData.m_settings._shadowsParams._cascadeBaseBias = { 0.001f, 0.002f, 0.01f, 0.05f };
    m_sceneData.m_settings._shadowsParams._cascadeSlopeBias = { 4.0f, 8.0f, 15.0f, 30.0f };
    m_sceneData.m_settings._shadowsParams._PCF = 2.0f;
    m_sceneData.m_settings._shadowsParams._textelScale = 1.0f;
    m_sceneData.m_settings._vewportParams._moveSpeed = 10.0f;
    m_sceneData.m_settings._tonemapParams._ev100 = 0.5f;
    m_sceneData.m_settings._tonemapParams._gamma = 2.2f;

    auto rendomVector = []() ->math::float3
        {
            return { math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0) };
        };

    ObjectHandle linear_sampler_h = m_sceneData.m_globalResources.get("linear_sampler_repeat");
    ASSERT(linear_sampler_h.isValid(), "must be valid");
    renderer::SamplerState* sampler = objectFromHandle<renderer::SamplerState>(linear_sampler_h);

    ObjectHandle default_metalness_h = m_sceneData.m_globalResources.get("default_metalness");
    ASSERT(default_metalness_h.isValid(), "must be valid");
    renderer::Texture2D* default_metalness = objectFromHandle<renderer::Texture2D>(default_metalness_h);

    ObjectHandle default_roughness_h = m_sceneData.m_globalResources.get("default_roughness");
    ASSERT(default_roughness_h.isValid(), "must be valid");
    renderer::Texture2D* default_roughness = objectFromHandle<renderer::Texture2D>(default_roughness_h);

    ObjectHandle default_normal_h = m_sceneData.m_globalResources.get("default_normal");
    ASSERT(default_normal_h.isValid(), "must be valid");
    renderer::Texture2D* default_normal = objectFromHandle<renderer::Texture2D>(default_normal_h);

    ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
    ASSERT(uv_h.isValid(), "must be valid");
    renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

    ObjectHandle default_black_h = m_sceneData.m_globalResources.get("default_black");
    ASSERT(default_black_h.isValid(), "must be valid");
    renderer::Texture2D* default_black = objectFromHandle<renderer::Texture2D>(default_black_h);

    {
        addDirectionLightComponent({ 30.f, 90.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, "SunLight");
        addPointLightComponent({ 0.1f, 0.5f, -1.0f }, 5.0f, { 1.f, 1.f, 1.f, 1.f }, "PointLight0");
        addPointLightComponent({ 1.0f, 0.5f, 0.5f }, 5.0f, { 1.f, 0.f, 0.f, 1.f }, "PointLight1");
        addSpotLightComponent({ -5.f, 1.f, 1.f }, 8.f, 30.f, { 1.f, 1.f, 1.f, 1.f }, "SpotLight0");
    }

    {
        scene::Model::LoadPolicy policy;
        scene::Model* nodeCube = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("cube.fbx", policy, resource::ModelFileLoader::SkipMaterial | resource::ModelFileLoader::Optimization);
        nodeCube->setPosition(scene::TransformMode::Local, { 3.f, 1.f, -1.f });
        nodeCube->m_name = "cube.fbx";
        addNode(nodeCube);

        scene::Material* material = new scene::Material(m_device, scene::MaterialShadingModel::PBR_MetallicRoughness);
        material->setProperty("BaseColor", loadTexture2D("DiamondPlate008C_1K/DiamondPlate008C_1K-PNG_Color.png", true));
        material->setProperty("Normals", loadTexture2D("DiamondPlate008C_1K/DiamondPlate008C_1K-PNG_NormalGL.png", true));
        material->setProperty("Roughness", loadTexture2D("DiamondPlate008C_1K/DiamondPlate008C_1K-PNG_Roughness.png", true));
        material->setProperty("Metalness", loadTexture2D("DiamondPlate008C_1K/DiamondPlate008C_1K-PNG_Metalness.png", true));
        material->setProperty("Displacement", loadTexture2D("DiamondPlate008C_1K/DiamondPlate008C_1K-PNG_Displacement.png", true));
        material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        nodeCube->addComponent(material);
    }

    u32 objects = 10;
    for (u32 i = 0; i < objects; ++i)
    {
        scene::Model::LoadPolicy policy;
        policy.scaleFactor = 0.5f;

        scene::Model* nodeCube = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("cube.fbx", policy, resource::ModelFileLoader::SkipMaterial | resource::ModelFileLoader::Optimization);
        nodeCube->m_name = std::format("cube{}", i);
        nodeCube->setPosition(scene::TransformMode::Local, randomVector(-5.f, 5.f));
        addNode(nodeCube);

        scene::Material* material = new scene::Material(m_device, scene::MaterialShadingModel::PBR_MetallicRoughness);
        material->setProperty("BaseColor", loadTexture2D("Bricks054/Bricks054_1K-PNG_Color.png", true));
        material->setProperty("Normals", loadTexture2D("Bricks054/Bricks054_1K-PNG_NormalGL.png", true));
        material->setProperty("Roughness", loadTexture2D("Bricks054/Bricks054_1K-PNG_Roughness.png", true));
        material->setProperty("Metalness", default_metalness);
        material->setProperty("Displacement", loadTexture2D("Bricks054/Bricks054_1K-PNG_Displacement.png", true));
        material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        nodeCube->addComponent(material);
    }

    {
        scene::Model::LoadPolicy policy;
        scene::Model* nodePlane = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("plane.fbx", policy, resource::ModelFileLoader::SkipMaterial | resource::ModelFileLoader::Optimization);
        nodePlane->m_name = "plane.fbx";
        addNode(nodePlane);

        scene::Material* material = new scene::Material(m_device, scene::MaterialShadingModel::PBR_MetallicRoughness);
        material->setProperty("BaseColor", loadTexture2D("PavingStones142_1K/PavingStones142_1K-PNG_Color.png", true));
        material->setProperty("Normals", loadTexture2D("PavingStones142_1K/PavingStones142_1K-PNG_NormalGL.png", true));
        material->setProperty("Roughness", loadTexture2D("PavingStones142_1K/PavingStones142_1K-PNG_Roughness.png", true));
        material->setProperty("Metalness", default_metalness);
        material->setProperty("Displacement", loadTexture2D("PavingStones142_1K/PavingStones142_1K-PNG_Displacement.png", true));
        material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        nodePlane->addComponent(material);
    }

    {
        scene::Model::LoadPolicy policy;
        policy.scaleFactor = 1.0f;
        scene::Model* nodeField = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("big_field.dae", policy, resource::ModelFileLoader::SkipMaterial | resource::ModelFileLoader::Optimization);
        nodeField->m_name = "big_field";
        nodeField->setPosition(scene::TransformMode::Local, { 5.0f, -0.03f, 0.0f });
        nodeField->setRotation(scene::TransformMode::Local, { 90.f, 0.0f, 0.0f });
        addNode(nodeField);

        scene::Material* material = new scene::Material(m_device, scene::MaterialShadingModel::PBR_MetallicRoughness);
        material->setProperty("BaseColor", uv_h);
        material->setProperty("Normals", loadTexture2D("Bricks054/Bricks054_1K-PNG_NormalGL.png", true));
        material->setProperty("Roughness", default_roughness);
        material->setProperty("Metalness", default_metalness);
        material->setProperty("Displacement", default_black);
        material->setProperty("DiffuseColor", math::float4{ 1.0, 1.0, 1.0, 1.0 });
        nodeField->addComponent(material);
    }

    {
        scene::SceneNode* skyboxNode = new scene::SceneNode();
        skyboxNode->m_name = "Skybox";
        addNode(skyboxNode);

        scene::Skybox* skybox = new scene::Skybox(m_device);
        skyboxNode->addComponent(skybox);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("BaseColor", loadTexture2D("Skybox/DaySkyHDRI026A_4K-HDR.dds", false));
        material->setProperty("pipelineID", 0U);
        skyboxNode->addComponent(material);
    }
}

void EditorScene::editor_loadDebug()
{
    ObjectHandle linear_sampler_h = m_sceneData.m_globalResources.get("linear_sampler_repeat");
    ASSERT(linear_sampler_h.isValid(), "must be valid");
    renderer::SamplerState* sampler = objectFromHandle<renderer::SamplerState>(linear_sampler_h);

    ObjectHandle default_black_h = m_sceneData.m_globalResources.get("default_black");
    ASSERT(default_black_h.isValid(), "must be valid");
    renderer::Texture2D* defaultBlack = objectFromHandle<renderer::Texture2D>(default_black_h);

    ObjectHandle uv_h = m_sceneData.m_globalResources.get("uv_grid");
    ASSERT(uv_h.isValid(), "must be valid");
    renderer::Texture2D* uvGrid = objectFromHandle<renderer::Texture2D>(uv_h);

    scene::SceneNode* editorNode = new scene::SceneNode();
    editorNode->m_name = "Editor";
    addNode(editorNode);

    {
        scene::SceneNode* node = new scene::SceneNode();
        node->m_name = "Grid";
        editorNode->addChild(node);

        scene::Mesh* grid = scene::MeshHelper::createGrid(m_device, 100.0f, 100, 100);
        node->addComponent(grid);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
        material->setProperty("pipelineID", 2U);
        material->setProperty("DiffuseColor", math::float4{ 0.7, 0.7, 0.7, 1.0 });
        node->addComponent(material);
    }

    {
        scene::SceneNode* node = new scene::SceneNode();
        node->m_name = "LineX";
        editorNode->addChild(node);

        scene::Mesh* line = scene::MeshHelper::createLineSegment(m_device, { math::float3{-100, 0.0, 0}, math::float3{100, 0.0, 0} });
        node->addComponent(line);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
        material->setProperty("pipelineID", 2U);
        material->setProperty("DiffuseColor", math::float4{ 1.0, 0.0, 0.0, 1.0 });
        node->addComponent(material);
    }

    {
        scene::SceneNode* node = new scene::SceneNode();
        node->m_name = "LineY";
        editorNode->addChild(node);

        scene::Mesh* line = scene::MeshHelper::createLineSegment(m_device, { math::float3{0, -100, 0}, math::float3{0, 100, 0} });
        node->addComponent(line);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
        material->setProperty("pipelineID", 2U);
        material->setProperty("DiffuseColor", math::float4{ 0.0, 1.0, 0.0, 1.0 });
        node->addComponent(material);
    }

    {
        scene::SceneNode* node = new scene::SceneNode();
        node->m_name = "LineZ";
        editorNode->addChild(node);

        scene::Mesh* line = scene::MeshHelper::createLineSegment(m_device, { math::float3{0, 0.0, -100}, math::float3{0, 0.0, 100} });
        node->addComponent(line);

        scene::Material* material = new scene::Material(m_device);
        material->setProperty("materialID", toEnumType(scene::ScenePass::Debug));
        material->setProperty("pipelineID", 2U);
        material->setProperty("DiffuseColor", math::float4{ 0.0, 0.0, 1.0, 1.0 });
        node->addComponent(material);
    }
}
