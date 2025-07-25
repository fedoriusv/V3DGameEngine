#include "EditorScene.h"
#include "Utils/Logger.h"

#include "Resource/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"
#include "Resource/Loader/ShaderCompiler.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Geometry/GeometryPrimitives.h"

#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Renderer/ShaderProgram.h"

#include "RenderTechniques/RenderPipelineGBuffer.h"
#include "RenderTechniques/RenderPipelineOutline.h"
#include "RenderTechniques/RenderPipelineGammaCorrection.h"
#include "RenderTechniques/RenderPipelineUIOverlay.h"
#include "RenderTechniques/RenderPipelineComposite.h"
#include "RenderTechniques/RenderPipelineZPrepass.h"
#include "RenderTechniques/RenderPipelineFXAA.h"
#include "RenderTechniques/RenderPipelineTAA.h"
#include "RenderTechniques/RenderPipelineMBOIT.h"
#include "RenderTechniques/RenderPipelineSelectionStage.h"
#include "RenderTechniques/RenderPipelineDeferredLightingStage.h"
#include "RenderTechniques/RenderPipelineVolumeLightingStage.h"
#include "RenderTechniques/RenderPipelineDebug.h"

#include "Stream/StreamManager.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

s32 g_objectIDCounter = 1;

using namespace v3d;

EditorScene::RenderPipelineScene::RenderPipelineScene(scene::ModelHandler* modelHandler)
{
    new renderer::RenderPipelineZPrepassStage(this, modelHandler);
    new renderer::RenderPipelineGBufferStage(this, modelHandler);
    new renderer::RenderPipelineSelectionStage(this, modelHandler);
    new renderer::RenderPipelineDeferredLightingStage(this);
    new renderer::RenderPipelineVolumeLightingStage(this, modelHandler);
    //new renderer::RenderPipelineMBOITStage(this);
    new renderer::RenderPipelineOutlineStage(this);
    new renderer::RenderPipelineTAAStage(this);
    new renderer::RenderPipelineDebugStage(this, modelHandler);
    new renderer::RenderPipelineGammaCorrectionStage(this);
    new renderer::RenderPipelineUIOverlayStage(this, nullptr);
}

EditorScene::RenderPipelineScene::~RenderPipelineScene()
{
}

EditorScene::EditorScene() noexcept
    : m_device()

    , m_modelHandler(new scene::ModelHandler())
    , m_UIHandler(nullptr)
    , m_inputHandler(new event::InputEventHandler())
    , m_gameHandler(new event::GameEventHandler())
    , m_gameEventRecevier(new event::GameEventReceiver([](event::GameEvent* event)
        {
            delete event;
        }))

    , m_mainPipeline(m_modelHandler)

    , m_camera(new scene::CameraEditorHandler(std::make_unique<scene::Camera>()))
    , m_contentList(nullptr)
    , m_frameCounter(0)

    , m_activeIndex(k_emptyIndex)

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
                        renderer::RenderPipelineOutlineStage::MappedData* readback_objectIDData = objectFromHandle<renderer::RenderPipelineOutlineStage::MappedData>(selectedObject);
                        m_activeIndex = (readback_objectIDData->_ptr) ? readback_objectIDData->_ptr[0] - 1 : k_emptyIndex;

                        m_gameEventRecevier->sendEvent(new EditorSelectionEvent(m_activeIndex));
                    }
                }
                m_camera->handleInputEventCallback(m_inputHandler, event);
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
                        m_camera->setTarget(m_sceneData.m_generalList[m_activeIndex]->_transform.getPosition()); //TODO
                    }
                }
                m_camera->handleInputEventCallback(m_inputHandler, event);
            }
        }
    );

    m_gameHandler->bind([this](const event::GameEvent* event, event::GameEvent::GameEventType type, u64 ID)
        {
            if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
            {
                const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
                m_activeIndex = selectionEvent->_selectedIndex;
            }
        }
    );

    m_gameEventRecevier->attach(m_gameHandler);

    resource::ResourceManager::createInstance();
    resource::ResourceManager::getInstance()->addPath("../../../../examples/uieditor/data/textures/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/textures/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/models/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/shaders/");

    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/_suntemple/");

    m_frameState.resize(3, {});
    m_stateIndex = 0;
}

EditorScene::~EditorScene()
{
    m_gameEventRecevier->dettach(m_gameHandler);

    delete m_modelHandler;
    //delete m_UIHandler;
    
    delete m_gameEventRecevier;
    delete m_inputHandler;
    delete m_gameHandler;

    delete m_camera;
}

void EditorScene::create(renderer::Device* device, const math::Dimension2D& viewportSize)
{
    m_device = device;
    m_currentViewportRect = math::Rect(0, 0, viewportSize._width, viewportSize._height);
    m_camera->setPerspective(m_vewportParams._fov, viewportSize, m_vewportParams._near, m_vewportParams._far);
    m_camera->setMoveSpeed(0.5f);
    m_camera->setRotationSpeed(25.0f);
    m_camera->setTarget({ 0.f, 0.f, 0.f });
    m_camera->setPosition({ 0.f, 0.25f, -1.f });

    loadResources();

    m_sceneData.m_viewportState._viewpotSize = { (u32)viewportSize._width, (u32)viewportSize._height };
    m_sceneData.m_diectionalLightState._color = { 1.f, 1.f, 1.f, 1.f };
    m_sceneData.m_diectionalLightState._intensity = 1.0;
    m_sceneData.m_diectionalLightState._temperature = 100.0;
    m_sceneData.m_diectionalLightState._attenuation = 1.0;
    m_mainPipeline.create(m_device, m_sceneData, m_frameState[m_stateIndex]);

    m_gameEventRecevier->sendEvent(new EditorSelectionEvent(k_emptyIndex));
}

void EditorScene::destroy()
{
    m_mainPipeline.destroy(m_device, m_sceneData, m_frameState[m_stateIndex]);
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
    renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    m_sceneData.m_renderState.m_cmdList = cmdList;

    m_camera->update(dt);
    m_sceneData.m_viewportState._camera = m_camera;

    s32 posX = (s32)m_inputHandler->getAbsoluteCursorPosition()._x - (s32)m_currentViewportRect.getLeftX();
    posX = (posX < 0) ? 0 : posX;
    s32 posY = (s32)m_inputHandler->getAbsoluteCursorPosition()._y - (s32)m_currentViewportRect.getTopY();
    posY = (posY < 0) ? 0 : posY;

    scene::ViewportState::ViewportBuffer& viewportState = m_sceneData.m_viewportState._viewportBuffer;
    viewportState.prevProjectionMatrix = viewportState.projectionMatrix;
    viewportState.prevViewMatrix = viewportState.viewMatrix;
    viewportState.prevCameraJitter = viewportState.cameraJitter;
    viewportState.projectionMatrix = m_camera->getCamera().getProjectionMatrix();
    viewportState.invProjectionMatrix = m_camera->getCamera().getProjectionMatrix().getInversed();
    viewportState.viewMatrix = m_camera->getCamera().getViewMatrix();
    viewportState.invViewMatrix = m_camera->getCamera().getViewMatrix().getInversed();
    viewportState.cameraJitter = scene::CameraHandler::calculateJitter(m_frameCounter, m_sceneData.m_viewportState._viewpotSize);
    viewportState.cameraPosition = { m_camera->getPosition().getX(), m_camera->getPosition().getY(), m_camera->getPosition().getZ(), 0.f };
    viewportState.viewportSize = { (f32)m_sceneData.m_viewportState._viewpotSize._width, (f32)m_sceneData.m_viewportState._viewpotSize._height };
    viewportState.clipNearFar = { m_camera->getNear(), m_camera->getFar() };
    viewportState.random = { math::random<f32>(0.f, 0.1f),math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f) };
    viewportState.cursorPosition = { (f32)posX, (f32)posY };
    viewportState.time = utils::Timer::getCurrentTime();

    m_modelHandler->visibilityTest(m_sceneData);
    if (m_activeIndex != k_emptyIndex)
    {
        m_sceneData.m_lists[toEnumType(scene::MaterialType::Selected)].push_back(m_sceneData.m_generalList[m_activeIndex]);
    }

    m_mainPipeline.prepare(m_device, m_sceneData, m_frameState[m_stateIndex]);
}

void EditorScene::postRender()
{
    m_mainPipeline.execute(m_device, m_sceneData, m_frameState[m_stateIndex]);
}

void EditorScene::submitRender()
{
    m_device->submit(m_sceneData.m_renderState.m_cmdList, true);
    m_device->destroyCommandList(m_sceneData.m_renderState.m_cmdList);
    m_sceneData.m_renderState.m_cmdList = nullptr;

    m_stateIndex = 0;//(m_stateIndex + 1) % m_states.size();
}

void EditorScene::modifyObject(const scene::Transform& transform)
{
    if (m_activeIndex != k_emptyIndex)
    {
        m_sceneData.m_generalList[m_activeIndex]->_transform = transform;
    }
}

void EditorScene::selectObject(u32 i)
{
    m_activeIndex = i;
    m_gameEventRecevier->sendEvent(new EditorSelectionEvent(m_activeIndex));
}

void EditorScene::onChanged(const v3d::math::Rect& viewport)
{
    if (viewport != m_currentViewportRect)
    {
        if (m_currentViewportRect.getWidth() != viewport.getWidth() || m_currentViewportRect.getHeight() != viewport.getHeight())
        {
            m_sceneData.m_viewportState._viewpotSize = { (u32)viewport.getWidth(), (u32)viewport.getHeight() };
        }

        m_currentViewportRect = viewport;
    }
}

void EditorScene::onChanged(const math::Matrix4D& view)
{
    m_camera->setViewMatrix(view);
    m_camera->setTarget(m_camera->getPosition() + m_camera->getCamera().getForwardVector() * 2.f);
    m_camera->update(0.f);
}

const renderer::Texture2D* EditorScene::getOutputTexture() const
{
    ObjectHandle final = m_sceneData.m_globalResources.get("final");
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
    return &m_camera->getCamera();
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
    static auto loadTexture2D = [this](renderer::CmdListRender* cmdList, const std::string& file) -> renderer::Texture2D*
        {
            resource::Bitmap* bitmap = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>(file);
            if (bitmap)
            {
                renderer::Texture2D* texture = new renderer::Texture2D(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
                    bitmap->getFormat(), { bitmap->getDimension()._width, bitmap->getDimension()._height }, 1, bitmap->getMipmapsCount(), file);
                ASSERT(texture, "not valid");

                cmdList->uploadData(texture, bitmap->getSize(), bitmap->getBitmap());
                m_device->submit(cmdList, true);

                resource::ResourceManager::getInstance()->remove(bitmap);

                return texture;
            }

            return nullptr;
        };

    renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    renderer::Texture2D* default_black = loadTexture2D(cmdList, "default_black.dds");
    renderer::Texture2D* default_white = loadTexture2D(cmdList, "default_white.dds");
    renderer::Texture2D* default_normal = loadTexture2D(cmdList, "default_normal.dds");
    renderer::Texture2D* default_material = loadTexture2D(cmdList, "default_material.dds");
    renderer::Texture2D* uv_grid = loadTexture2D(cmdList, "uv_grid.dds");
    renderer::Texture2D* noise_blue = loadTexture2D(cmdList, "noise_blue.dds");
    renderer::Texture2D* tiling_noise = loadTexture2D(cmdList, "good64x64tilingnoisehighfreq.dds");

    m_sceneData.m_globalResources.bind("default_black", default_black);
    m_sceneData.m_globalResources.bind("default_white", default_white);
    m_sceneData.m_globalResources.bind("default_normal", default_normal);
    m_sceneData.m_globalResources.bind("default_material", default_material);
    m_sceneData.m_globalResources.bind("uv_grid", uv_grid);
    m_sceneData.m_globalResources.bind("noise_blue", noise_blue);
    m_sceneData.m_globalResources.bind("tiling_noise", tiling_noise);

    renderer::SamplerState* linear_sampler_repeat = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_repeat->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    renderer::SamplerState* linear_sampler_mirror = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_mirror->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    renderer::SamplerState* linear_sampler_clamp = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    linear_sampler_clamp->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    renderer::SamplerState* nearest_sampler_repeat = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    nearest_sampler_repeat->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    renderer::SamplerState* nearest_sampler_clamp = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    nearest_sampler_clamp->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    m_sceneData.m_globalResources.bind("linear_sampler_repeat", linear_sampler_repeat);
    m_sceneData.m_globalResources.bind("linear_sampler_mirror", nearest_sampler_repeat);
    m_sceneData.m_globalResources.bind("linear_sampler_clamp", linear_sampler_clamp);
    m_sceneData.m_globalResources.bind("nearest_sampler_repeat", nearest_sampler_repeat);
    m_sceneData.m_globalResources.bind("nearest_sampler_clamp", nearest_sampler_clamp);

    test_loadCubes(cmdList, 20, 20);
    editor_loadDebug(cmdList);

    m_device->submit(cmdList, true);
    m_device->destroyCommandList(cmdList);
}

void EditorScene::test_loadCubes(renderer::CmdListRender* cmdList, u32 countOpaque, u32 countTransparency)
{
    auto rendomVector = []() ->math::float3
        {
            return { math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0) };
        };

    //load texture
    static auto loadTexture2D = [this](renderer::CmdListRender* cmdList, const std::string& file, bool generateMips) -> renderer::Texture2D*
        {
            resource::ImageLoaderFlags flags = 0;
            if (generateMips)
            {
                flags |= resource::ImageLoaderFlag::ImageLoader_GenerateMipmaps;
            }
            resource::Bitmap* bitmap = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>(file, flags);
            if (bitmap)
            {
                renderer::Texture2D* texture = new renderer::Texture2D(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
                    bitmap->getFormat(), { bitmap->getDimension()._width, bitmap->getDimension()._height }, 1, bitmap->getMipmapsCount(), file);
                ASSERT(texture, "not valid");

                cmdList->uploadData(texture, bitmap->getSize(), bitmap->getBitmap());
                m_device->submit(cmdList, true);

                resource::ResourceManager::getInstance()->remove(bitmap);

                return texture;
            }

            return nullptr;
        };

    //load model
    static auto loadModel = [this](renderer::CmdListRender* cmdList, const std::string& file) ->  scene::Model*
        {
            resource::ModelResource* modelRes = resource::ResourceManager::getInstance()->load<resource::ModelResource, resource::ModelFileLoader>(file);

            scene::Model* model = scene::ModelHelper::createModel(m_device, cmdList, modelRes);
            m_device->submit(cmdList, true);

            resource::ResourceManager::getInstance()->remove(modelRes);

            return model;
        };

    scene::Model* cube = loadModel(cmdList, "cube.dae");
    renderer::Texture2D* color = loadTexture2D(cmdList, "Bricks054_1K-PNG_Color.png", true);
    renderer::Texture2D* normals = loadTexture2D(cmdList, "Bricks054_1K-PNG_NormalDX.png", true);
    renderer::Texture2D* roughness = loadTexture2D(cmdList, "Bricks054_1K-PNG_Roughness.png", true);

    ObjectHandle linear_sampler_h = m_sceneData.m_globalResources.get("linear_sampler_repeat");
    ASSERT(linear_sampler_h.isValid(), "must be valid");
    renderer::SamplerState* sampler = objectFromHandle<renderer::SamplerState>(linear_sampler_h);

    for (u32 i = 0; i < countOpaque; ++i)
    {
        math::float3 pos = rendomVector() * 1.0f;
        math::float3 scale = rendomVector() * 10.f;

        scene::DrawInstanceDataState* data = new scene::DrawInstanceDataState;
        data->_geometry._ID = "cube";
        data->_geometry._idxBuffer = cube->m_geometry[0]._LODs[0]->m_indexBuffer;
        data->_geometry._vtxBuffer = cube->m_geometry[0]._LODs[0]->m_vertexBuffer[0];
        data->_material._type = scene::MaterialType::Opaque;
        data->_material._sampler = sampler;
        data->_material._albedo = color;
        data->_material._normals = normals;
        data->_material._material = roughness;
        data->_material._tint = { 1.0, 1.0, 1.0, 1.0 };
        data->_transform.setPosition({ pos._x, pos._y, pos._z });
        data->_transform.setScale({ scale._x, scale._x, scale._x });
        data->_prevTransform = data->_transform;
        data->_objectID = g_objectIDCounter++;
        data->_pipelineID = 0;

        m_sceneData.m_generalList.push_back(data);
    }

    for (u32 i = 0; i < countTransparency; ++i)
    {
        math::float3 pos = rendomVector() * 1.0f;
        math::float3 scale = rendomVector() * 10.f;
        math::float3 tint = rendomVector();

        scene::DrawInstanceDataState* data = new scene::DrawInstanceDataState;
        data->_geometry._ID = "cube";
        data->_geometry._idxBuffer = cube->m_geometry[0]._LODs[0]->m_indexBuffer;
        data->_geometry._vtxBuffer = cube->m_geometry[0]._LODs[0]->m_vertexBuffer[0];
        data->_material._type = scene::MaterialType::Transparency;
        data->_material._sampler = sampler;
        data->_material._albedo = color;
        data->_material._normals = normals;
        data->_material._material = roughness;
        data->_material._tint = { tint._x, tint._y, tint._z, 0.5 };
        data->_transform.setPosition({ pos._x, pos._y, pos._z });
        data->_transform.setScale({ scale._x, scale._x, scale._x });
        data->_prevTransform = data->_transform;
        data->_objectID = g_objectIDCounter++;
        data->_pipelineID = 0;

        m_sceneData.m_generalList.push_back(data);
    }
}

void EditorScene::editor_loadDebug(renderer::CmdListRender* cmdList)
{
    {
        scene::Mesh* prim = scene::Primitives::createGrid(m_device, cmdList, 10.0f, 64, 64);
        scene::DrawInstanceDataState* grid = new scene::DrawInstanceDataState;
        grid->_geometry._ID = "EditorGrid";
        grid->_geometry._idxBuffer = prim->m_indexBuffer;
        grid->_geometry._vtxBuffer = prim->m_vertexBuffer[0];
        grid->_material._type = scene::MaterialType::Debug;
        grid->_material._sampler = nullptr;
        grid->_material._albedo = nullptr;
        grid->_material._normals = nullptr;
        grid->_material._material = nullptr;
        grid->_material._tint = { 0.5, 0.5, 0.5, 1.0 };
        grid->_transform.setPosition({ 0.f, 0.f, 0.f });
        grid->_transform.setScale({ 1.f, 1.f, 1.f });
        grid->_prevTransform = grid->_transform;
        grid->_objectID = g_objectIDCounter++;
        grid->_pipelineID = 1;

        m_sceneData.m_generalList.push_back(grid);
    }

    {
        scene::Mesh* directionalLightDebug = scene::Primitives::createLine(m_device, cmdList, { { 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f } });
        scene::DrawInstanceDataState* line = new scene::DrawInstanceDataState;
        line->_geometry._ID = "EditorDirectionalLight";
        line->_geometry._idxBuffer = directionalLightDebug->m_indexBuffer;
        line->_geometry._vtxBuffer = directionalLightDebug->m_vertexBuffer[0];
        line->_material._type = scene::MaterialType::Debug;
        line->_material._sampler = nullptr;
        line->_material._albedo = nullptr;
        line->_material._normals = nullptr;
        line->_material._material = nullptr;
        line->_material._tint = { 1.0, 1.0, 0.f, 1.0 };
        line->_transform.setPosition({ 0.f, 0.f, 0.f });
        line->_transform.setScale({ 1.f, 1.f, 1.f });
        line->_prevTransform = line->_transform;
        line->_objectID = g_objectIDCounter++;
        line->_pipelineID = 1;

        m_sceneData.m_generalList.push_back(line);
    }
}
