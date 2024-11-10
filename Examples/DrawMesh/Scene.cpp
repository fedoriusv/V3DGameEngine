#include "Scene.h"
#include "Scene/CameraArcballHelper.h"
#include "Scene/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"

#define VULKAN_ONLY 0

namespace app
{

using namespace v3d;

Scene::Scene(v3d::renderer::Device* device, renderer::Swapchain* swapchain) noexcept
    : m_Device(device)
    , m_Swapchain(swapchain)
    , m_CmdList(m_Device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask))

    , m_CurrentState(States::StateInit)

    , m_Camera(nullptr)
    , m_Render(nullptr)
{
    resource::ResourceManager::createInstance();
    resource::ResourceManager::getInstance()->addPath("../../../../examples/drawmesh/data/");
}

Scene::~Scene()
{
    resource::ResourceManager::getInstance()->clear();
    resource::ResourceManager::freeInstance();
}

void Scene::Run(f32 dt)
{
    switch (m_CurrentState)
    {
    case States::StateInit:
    {
        Init();
        m_CurrentState = States::StateLoad;
    }

    case States::StateLoad:
    {
        Load();

        m_CurrentState = States::StateDraw;
    }

    case States::StateDraw:
    {
        Draw(dt);
    }

    case States::StateExit:
    default:
    {
        Exit();
    }
    }
}

void Scene::SendExitSignal()
{
    m_CurrentState = States::StateExit;
}

void Scene::Init()
{
    //init camera
    m_Camera = new scene::CameraArcballHelper(new scene::Camera(math::Vector3D(0.0f, 0.0f, 0.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), 8.0f, 4.0f, 20.0f);
    m_Camera->setPerspective(45.0f, m_Swapchain->getBackbufferSize(), 1.f, 50.f);
    m_Camera->setRotation(math::Vector3D(0.0f, -90.0f, 0.0f));
}

void Scene::Load()
{
    //load resources
    LoadVoyager();

    {
#if VULKAN_ONLY
        const renderer::Shader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.vert");
        const renderer::Shader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.frag");
#else
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.hlsl", "main_vs");
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.hlsl", "main_ps");
#endif
        m_Render = new app::TextureRender(m_Device, *m_CmdList, m_Swapchain, {vertShader, fragShader}, renderer::VertexInputAttributeDesc());
    }
}

void Scene::LoadVoyager()
{
    Model* voyager = new Model();

    {
#if defined(PLATFORM_ANDROID)
        resource::Image* image = resource::ResourceManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("models/voyager/voyager_astc_8x8_unorm.ktx");
        voyager->m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
#else
        resource::Bitmap* image = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("models/voyager/voyager_bc3_unorm.ktx");
        voyager->m_Sampler = new renderer::SamplerState(m_Device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
#endif
        voyager->m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);
        voyager->m_Texture = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
            image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), 1);
        m_CmdList->uploadData(voyager->m_Texture.get(), image->getSize(), image->getBitmap());
    }

    {
        scene::Model* model = resource::ResourceManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("models/voyager/voyager.dae",
            resource::ModelFileLoader::ModelLoaderFlag::FlipYTextureCoord | resource::ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent | resource::ModelFileLoader::ModelLoaderFlag::SkipMaterial);

        //renderer::VertexBuffer* vertexBuffer = new renderer::VertexBuffer(m_Device, renderer::Buffer_Read, size);
        //ASSERT(vertexBuffer, "nullptr");
        //m_buffers.push_back(utils::IntrusivePointer<renderer::StreamBuffer>(vertexBuffer));

        //if (staticMesh->getIndexCount() > 0)
        //{
        //    u32 count = staticMesh->getIndexCount();
        //    const void* data = staticMesh->getIndexData();
        //    renderer::StreamIndexBufferType type = staticMesh->getIndexType();

        //    renderer::IndexStreamBuffer* indexBuffer = cmdList->createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, type, count, data);
        //    ASSERT(indexBuffer, "nullptr");
        //    m_buffers.push_back(utils::IntrusivePointer<renderer::StreamBuffer>(indexBuffer));

        //    renderer::DrawProperties props = { 0, count, 0, 1, true };
        //    m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(indexBuffer, 0, vertexBuffer, 0, 0), props));
        //}
        //else
        //{
        //    renderer::DrawProperties props = { 0, staticMesh->getVertexCount(), 0, 1, false };
        //    m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(vertexBuffer, 0, 0), props));
        //}
    }

    m_Models.push_back(voyager);
}

void Scene::Draw(f32 dt)
{
    //Frame
    m_Swapchain->beginFrame();

    m_Camera->update(dt);

    for (auto& model : m_Models)
    {
        m_Render->updateParameters(*m_CmdList, [this](TextureUniformParameters& params) -> void
            {
                //vs
                params._constantBufferVS._projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
                params._constantBufferVS._viewMatrix = m_Camera->getCamera().getViewMatrix();
                params._constantBufferVS._modelMatrix.setTranslation(math::Vector3D(0, 0, 0));
                params._constantBufferVS._normalMatrix = params._constantBufferVS._modelMatrix;
                params._constantBufferVS._normalMatrix.makeTransposed();

                params._constantBufferFS._lightPos = math::Vector4D(25.0f, 0.0f, 5.0f, 1.0f);

                //ps
                //params._texture = m_CurrentModel->m_Texture.get();
                //params._sampler = m_CurrentModel->m_Sampler.get();
            });

        m_Render->process(*m_CmdList, model->m_Props);
    }

    m_Device->submit(m_CmdList);

    m_Swapchain->endFrame();
    m_Swapchain->presentFrame();
}

void Scene::Exit()
{
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