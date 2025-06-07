#include "EditorScene.h"
#include "Utils/Logger.h"

#include "Resource/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"
#include "Resource/Loader/ShaderCompiler.h"

#include "Scene/Model.h"
#include "Scene/Geometry/Mesh.h"

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
#include "RenderTechniques/RenderPipelineTransparency.h"
#include "RenderTechniques/RenderPipelineZPrepass.h"
#include "RenderTechniques/RenderPipelineFXAA.h"

#include "Stream/StreamManager.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

namespace v3d
{

scene::Transform g_modelTransform;
u32 g_activeIndex = 0;

EditorScene::RenderPipelineScene::RenderPipelineScene()
{
    new renderer::RenderPipelineZPrepassStage(this);
    new renderer::RenderPipelineGBufferStage(this);
    new renderer::RenderPipelineCompositionStage(this);
    //new renderer::RenderPipelineTransparencyStage(this);
    new renderer::RenderPipelineOutlineStage(this);
    //new renderer::RenderPipelineFXAAStage(this);
    new renderer::RenderPipelineGammaCorrectionStage(this);
    new renderer::RenderPipelineUIOverlayStage(this);
}

EditorScene::RenderPipelineScene::~RenderPipelineScene()
{
}

EditorScene::EditorScene() noexcept
    : m_device()
    , m_camera(new scene::CameraEditorHandler(std::make_unique<scene::Camera>()))
    , m_contentList(nullptr)
{
    InputEventHandler::bind([this](const event::MouseInputEvent* event)
        {
            if (m_currentViewportRect.isPointInside({ (f32)this->getAbsoluteCursorPosition()._x, (f32)this->getAbsoluteCursorPosition()._y }))
            {
                m_camera->handleInputEventCallback(this, event);
            }
        }
    );

    InputEventHandler::bind([this](const event::KeyboardInputEvent* event)
        {
            if (m_currentViewportRect.isPointInside({ (f32)this->getAbsoluteCursorPosition()._x, (f32)this->getAbsoluteCursorPosition()._y }))
            {
                if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
                {
                    if (event->_key == event::KeyCode::KeyKey_F) //focus on selected object
                    {
                        m_camera->setTarget(g_modelTransform.getPosition()); //TODO
                    }
                }
                m_camera->handleInputEventCallback(this, event);
            }
        }
        );

    resource::ResourceManager::createInstance();
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/textures/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/models/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/shaders/");

    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/_suntemple/");

    m_states.resize(3, {});
    m_stateIndex = 0;
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

    m_states[m_stateIndex].m_viewportState.m_viewpotSize = { (u32)viewportSize._width, (u32)viewportSize._height };
    m_pipeline.create(m_device, m_states[m_stateIndex]);
}

void EditorScene::destroy()
{
    m_pipeline.destroy(m_device, m_states[m_stateIndex]);
}

void EditorScene::preRender(f32 dt)
{
    renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    m_states[m_stateIndex].m_renderState.m_cmdList = cmdList;

    m_camera->update(dt);
    m_states[m_stateIndex].m_viewportState.m_camera = m_camera;

    s32 posX = (s32)this->getAbsoluteCursorPosition()._x - (s32)m_currentViewportRect.getLeftX();
    posX = (posX < 0) ? 0 : posX;
    s32 posY = (s32)this->getAbsoluteCursorPosition()._y - (s32)m_currentViewportRect.getTopY();
    posY = (posY < 0) ? 0 : posY;

    ViewportState::ViewportBuffer& viewportState = m_states[m_stateIndex].m_viewportState._viewportBuffer;
    viewportState.prevProjectionMatrix = viewportState.projectionMatrix;
    viewportState.prevViewMatrix = viewportState.viewMatrix;
    viewportState.projectionMatrix = m_camera->getCamera().getProjectionMatrix();
    viewportState.viewMatrix = m_camera->getCamera().getViewMatrix();
    viewportState.cameraPosition = { m_camera->getPosition().getX(), m_camera->getPosition().getY(), m_camera->getPosition().getZ(), 0.f };
    viewportState.viewportSize = { (f32)m_states[m_stateIndex].m_viewportState.m_viewpotSize._width, (f32)m_states[m_stateIndex].m_viewportState.m_viewpotSize._height };
    viewportState.cursorPosition = { (f32)posX, (f32)posY };
    viewportState.random = { math::random<f32>(0.f, 0.1f),math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f), math::random<f32>(0.f, 0.1f) };
    viewportState.time = utils::Timer::getCurrentTime();

    m_pipeline.prepare(m_device, m_states[m_stateIndex]);

    //if (g_activeIndex > -1)
    {
        m_states[m_stateIndex].m_data[g_activeIndex].m_transform = g_modelTransform;
    }
}

void EditorScene::postRender()
{
    m_pipeline.execute(m_device, m_states[m_stateIndex]);
}

void EditorScene::submitRender()
{
    m_device->submit(m_states[m_stateIndex].m_renderState.m_cmdList, true);
    m_device->destroyCommandList(m_states[m_stateIndex].m_renderState.m_cmdList);
    m_states[m_stateIndex].m_renderState.m_cmdList = nullptr;

    m_stateIndex = 0;//(m_stateIndex + 1) % m_states.size();
}

void EditorScene::test_setOpacity(f32 op)
{
    for (auto& v : m_states[m_stateIndex].m_data)
    {
        v.m_tint._w = op;
    }
}

void EditorScene::test_initContent(ui::WidgetListBox* list)
{
    m_contentList = list;
    for (auto& v : m_states[m_stateIndex].m_data)
    {
        m_contentList->addElement("Object_" + std::to_string(v.m_objectID));
    }
}

void EditorScene::test_selectItem(u32 i)
{
    g_activeIndex = i;
}

void EditorScene::onChanged(const v3d::math::Rect& viewport)
{
    if (viewport != m_currentViewportRect)
    {
        if (m_currentViewportRect.getWidth() != viewport.getWidth() || m_currentViewportRect.getHeight() != viewport.getHeight())
        {
            m_states[m_stateIndex].m_viewportState.m_viewpotSize = { (u32)viewport.getWidth(), (u32)viewport.getHeight() };
            m_pipeline.changed(m_device, m_states[m_stateIndex]);
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
    ObjectHandle final = m_states[m_stateIndex].m_globalResources.get("final");
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

void EditorScene::loadResources()
{
    static auto loadTexture2D = [this](renderer::CmdListRender* cmdList, const std::string& file) -> renderer::Texture2D*
        {
            resource::Bitmap* bitmap = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>(file);
            if (bitmap)
            {
                renderer::Texture2D* texture = new renderer::Texture2D(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
                    bitmap->getFormat(), { bitmap->getDimension()._width, bitmap->getDimension()._height }, bitmap->getMipmapsCount(), file);
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

    m_states[m_stateIndex].m_globalResources.bind("default_black", default_black);
    m_states[m_stateIndex].m_globalResources.bind("default_white", default_white);
    m_states[m_stateIndex].m_globalResources.bind("default_normal", default_normal);
    m_states[m_stateIndex].m_globalResources.bind("default_material", default_material);
    m_states[m_stateIndex].m_globalResources.bind("uv_grid", uv_grid);
    m_states[m_stateIndex].m_globalResources.bind("noise_blue", noise_blue);

    renderer::SamplerState* sampler = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);

    auto rendomVector = []() ->math::float3
        {
            return { math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0), math::random<f32>(0.0, 1.0) };
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

    scene::Model* model = loadModel(cmdList, "cube.dae");
    //scene::Model* sunTemple = loadModel(cmdList, "SunTemple.fbx");


    //for (u32 i = 0; i < sunTemple->m_geometry.size(); ++i)
    //{
    //    m_states[m_stateIndex].m_data.push_back(DrawInstanceData{
    //            sunTemple->m_geometry[0]._LODs[0]->m_indexBuffer,
    //            sunTemple->m_geometry[0]._LODs[0]->m_vertexBuffer[0]
    //        });

    //    m_states[m_stateIndex].m_data[i].m_tint = { 1, 1, 1, 1 };
    //    m_states[m_stateIndex].m_data[i].m_sampler = sampler;
    //    m_states[m_stateIndex].m_data[i].m_albedo = uv_grid;
    //    m_states[m_stateIndex].m_data[i].m_normals = default_normal;
    //    m_states[m_stateIndex].m_data[i].m_material = default_material;
    //    m_states[m_stateIndex].m_data[i].m_stageID = "gbuffer";
    //    m_states[m_stateIndex].m_data[i].m_pipelineID = 0;
    //    m_states[m_stateIndex].m_data[i].m_objectID = m_states[m_stateIndex].m_data.size();
    //}

    u32 countOpaque = 10;//sunTemple->m_geometry.size();
    for (u32 i = 0; i < countOpaque; ++i)
    {
        m_states[m_stateIndex].m_data.push_back(DrawInstanceData{
            model->m_geometry[0]._LODs[0]->m_indexBuffer,
            model->m_geometry[0]._LODs[0]->m_vertexBuffer[0]
            });

        math::float3 pos = rendomVector() * 0.75f;
        math::float3 scale = rendomVector() * 10.f;

        m_states[m_stateIndex].m_data[i].m_transform.setPosition({ pos._x, pos._y, pos._z });
        m_states[m_stateIndex].m_data[i].m_transform.setScale({ scale._x, scale._x, scale._x });
        m_states[m_stateIndex].m_data[i].m_prevTransform = m_states[m_stateIndex].m_data[i].m_transform;
        m_states[m_stateIndex].m_data[i].m_tint = { 1, 1, 1, 1 };
        m_states[m_stateIndex].m_data[i].m_sampler = sampler;
        m_states[m_stateIndex].m_data[i].m_albedo = uv_grid;
        m_states[m_stateIndex].m_data[i].m_normals = default_normal;
        m_states[m_stateIndex].m_data[i].m_material = default_material;
        m_states[m_stateIndex].m_data[i].m_stageID = "gbuffer";
        m_states[m_stateIndex].m_data[i].m_pipelineID = 0;
        m_states[m_stateIndex].m_data[i].m_objectID = m_states[m_stateIndex].m_data.size();
    }


    /*u32 countTr = 30;
    for (u32 i = countOpaque; i < countTr; ++i)
    {
        m_states[m_stateIndex].m_data.push_back(DrawInstanceData{
            model->m_geometry[0]._LODs[0]->m_indexBuffer,
            model->m_geometry[0]._LODs[0]->m_vertexBuffer[0]
            });

        math::float3 pos = rendomVector() * 0.3f;
        math::float3 scale = rendomVector() * 10.f;
        math::float3 color = rendomVector();

        m_states[m_stateIndex].m_data[i].m_transform.setPosition({ pos._x, pos._y, pos._z });
        m_states[m_stateIndex].m_data[i].m_transform.setScale({ scale._x, scale._x, scale._x });
        m_states[m_stateIndex].m_data[i].m_tint = { color._x, color._y, color._z, 0.5f };
        m_states[m_stateIndex].m_data[i].m_sampler = sampler;
        m_states[m_stateIndex].m_data[i].m_albedo = default_white;
        m_states[m_stateIndex].m_data[i].m_normals = default_normal;
        m_states[m_stateIndex].m_data[i].m_material = default_material;
        m_states[m_stateIndex].m_data[i].m_stageID = "transparency";
        m_states[m_stateIndex].m_data[i].m_pipelineID = 0;
        m_states[m_stateIndex].m_data[i].m_objectID = m_states[m_stateIndex].m_data.size();
    }*/

    m_device->submit(cmdList, true);
    m_device->destroyCommandList(cmdList);

    g_modelTransform = m_states[m_stateIndex].m_data[g_activeIndex].m_transform;
}

} //namespace v3d