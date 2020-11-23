#include "Scene.h"
#include "Scene/CameraArcballHelper.h"
#include "Scene/Model.h"

#include "Resource/Image.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"

#define VULKAN_ONLY 0

namespace app
{

using namespace v3d;

Scene::Scene(renderer::Context* context, const v3d::core::Dimension2D& size) noexcept
    : m_Context(context)
    , m_CommandList(nullptr)
    , m_Viewport(size)

    , m_CurrentState(States::StateInit)

    , m_Camera(nullptr)
    , m_Render(nullptr)

    , m_CurrentModel(nullptr)
{
    m_Voyager.m_Model = nullptr;
    m_Test.m_Model = nullptr;
}

Scene::~Scene()
{
}

bool Scene::Run()
{
    switch (m_CurrentState)
    {
    case States::StateInit:
    {
        Init();
        m_CurrentState = States::StateLoad;
        return true;
    }

    case States::StateLoad:
    {
        Load();

        m_CurrentState = States::StateDraw;
        return true;
    }

    case States::StateDraw:
    {
        Draw();
        return true;
    }

    case States::StateExit:
    default:
    {
        Exit();
        return false;
    }
    }

    return false;
}

void Scene::SendExitSignal()
{
    m_CurrentState = States::StateExit;
}

void Scene::Init()
{
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

    //init camera
    m_Camera = new scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), 8.0f, 4.0f, 20.0f);
    m_Camera->setPerspective(45.0f, m_Context->getBackbufferSize(), 1.f, 50.f);
    m_Camera->setRotation(core::Vector3D(0.0f, -90.0f, 0.0f));
}

void Scene::Load()
{
    //load resources
    resource::ResourceLoaderManager::getInstance()->addPath("examples/drawmesh/data/");
    const bool test = false;
    if (test)
    {
        LoadTest();
        m_CurrentModel = &m_Test;
    }
    else
    {
        LoadVoyager();
        m_CurrentModel = &m_Voyager;
    }

    {
#if VULKAN_ONLY
        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "shaders/mesh.vert", {});
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "shaders/mesh.frag", {});
#else
        renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "shaders/mesh.vs", {});
        renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Context, "shaders/mesh.ps", {});
#endif
        m_Render = new v3d::TextureRender(*m_CommandList, m_Viewport, { vertShader, fragShader }, m_CurrentModel->m_Model->getVertexInputAttribDescription(0, 0));
    }

    m_CommandList->submitCommands(true);
    m_CommandList->flushCommands();
}

void Scene::LoadVoyager()
{
    {
#if defined(PLATFORM_ANDROID)
        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("models/voyager/voyager_etc2_unorm.ktx");
#else
        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("models/voyager/voyager_bc3_unorm.ktx");
#endif
        m_Voyager.m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
        m_Voyager.m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);
        m_Voyager.m_Texture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
            image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, image->getRawData());
    }

    {
        scene::Model* model = resource::ResourceLoaderManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("models/voyager/voyager.dae", resource::ModelLoaderFlag::ModelLoaderFlag_FlipYTextureCoord);
        m_Voyager.m_Model = new scene::ModelHelper(*m_CommandList, { model });
    }
}

void Scene::LoadTest()
{
    resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("engine/data/textures/test_basetex.jpg");
    m_Test.m_Texture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
        image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, image->getRawData());
    m_Test.m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_Test.m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

    scene::Model* model = resource::ResourceLoaderManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("engine/data/models/test_blender.dae", resource::ModelLoaderFlag::ModelLoaderFlag_FlipYTextureCoord);
    m_Test.m_Model = new scene::ModelHelper(*m_CommandList, { model });
}

void Scene::Update()
{
    m_Render->updateParameters(*m_CommandList, [this](TextureUniformParameters& params) -> void
    {
        //vs
        params._constantBufferVS._projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
        params._constantBufferVS._modelMatrix.setTranslation(core::Vector3D(0, 0, 0));
        params._constantBufferVS._viewMatrix = m_Camera->getCamera().getViewMatrix();

        params._constantBufferFS._lightPos = core::Vector4D(25.0f, 0.0f, 5.0f, 1.0f);

        //ps
        params._texture = m_CurrentModel->m_Texture.get();
        params._sampler = m_CurrentModel->m_Sampler.get();
    });
}

void Scene::Draw()
{
    //Frame
    m_CommandList->beginFrame();

    m_Camera->update(0);
    Update();

    m_Render->process(*m_CommandList, m_CurrentModel->m_Model->getDrawStates());

    m_CommandList->endFrame();
    m_CommandList->presentFrame();

    m_CommandList->flushCommands();
}

void Scene::Exit()
{
    m_CommandList->submitCommands(true);
    m_CommandList->flushCommands();

    if (m_Render)
    {
        delete m_Render;
        m_Render = nullptr;
    }

    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = nullptr;
    }

    if (m_Voyager.m_Model)
    {
        delete m_Voyager.m_Model;
        m_Voyager = {};
    }

    if (m_Test.m_Model)
    {
        delete m_Test.m_Model;
        m_Test = {};
    }

    delete m_CommandList;
    resource::ResourceLoaderManager::getInstance()->clear();
}

void Scene::MouseCallback(Scene* scene, event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    if (scene->m_Camera)
    {
        scene->m_Camera->handlerMouseCallback(handler, event);
    }
}

void Scene::TouchCallback(Scene* scene, event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
    if (scene->m_Camera)
    {
        scene->m_Camera->handlerTouchCallback(handler, event);
    }
}

} //namespace app