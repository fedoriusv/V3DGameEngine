#include "Scene.h"
#include "Scene/CameraArcballHelper.h"
#include "Resource/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Scene/Model.h"
#include "Scene/Mesh.h"

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
    m_Device->destroyCommandList(m_CmdList);
    m_CmdList = nullptr;

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
        break;
    }

    case States::StateLoad:
    {
        Load();

        m_CurrentState = States::StateDraw;
        break;
    }

    case States::StateDraw:
    {
        Draw(dt);
        break;
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
    m_Camera = new scene::CameraArcballHelper(new scene::Camera(math::Vector3D(0.0f, 0.0f, 0.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), 8.0f, 4.0f, 80.0f);
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
        m_Render = new app::TextureRender(m_Device, *m_CmdList, m_Swapchain, {vertShader, fragShader}, m_Models.front()->m_InputAttrib);
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
            image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), image->getMipmapsCount());
        m_CmdList->uploadData(voyager->m_Texture.get(), image->getSize(), image->getBitmap());
    }

    {
        resource::ModelResource* modelRes = resource::ResourceManager::getInstance()->load<resource::ModelResource, resource::ModelFileLoader>("models/voyager/voyager.dae",
            resource::ModelFileLoader::ModelLoaderFlag::FlipYTextureCoord | resource::ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent | resource::ModelFileLoader::ModelLoaderFlag::SkipMaterial);

        scene::Model* model = scene::ModelHelper::createModel(m_Device, m_CmdList, modelRes);
        voyager->m_Model = model;
        for (auto& geom : model->m_geometry)
        {
            scene::Mesh* mesh = geom._LODs[0]; //first lod only
            if (mesh->m_indexBuffer)
            {
                v3d::renderer::GeometryBufferDesc desc(mesh->m_indexBuffer, 0, mesh->m_vertexBuffer[0], 0, mesh->getVertexAttribDesc()._inputBindings[0]._stride, 0);
                app::DrawProperties prop{ 0, mesh->m_indexBuffer->getIndicesCount(), 0, 1, true };
                voyager->m_Props.emplace_back(std::move(desc), prop);
                voyager->m_InputAttrib = mesh->getVertexAttribDesc();
            }
            else
            {
                v3d::renderer::GeometryBufferDesc desc(mesh->m_vertexBuffer[0], 0, mesh->getVertexAttribDesc()._inputBindings[0]._stride);
                app::DrawProperties prop{ 0, mesh->m_vertexBuffer[0]->getVerticesCount(), 0, 1, false };
                voyager->m_Props.emplace_back(std::move(desc), prop);
                voyager->m_InputAttrib = mesh->getVertexAttribDesc();
            }
        }

        resource::ResourceManager::getInstance()->remove(modelRes);
    }

    m_Models.push_back(voyager);

    m_Device->submit(m_CmdList, true);
}

void Scene::Draw(f32 dt)
{
    //Frame
    m_Swapchain->beginFrame();

    m_Camera->update(dt);

    for (auto& model : m_Models)
    {
        m_Render->updateParameters(*m_CmdList, [this, model](TextureUniformParameters& params) -> void
            {
                //vs
                params._constantBufferVS._projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
                params._constantBufferVS._viewMatrix = m_Camera->getCamera().getViewMatrix();
                params._constantBufferVS._modelMatrix.setTranslation(math::Vector3D(0, 0, 0));
                params._constantBufferVS._normalMatrix = params._constantBufferVS._modelMatrix;
                params._constantBufferVS._normalMatrix.makeTransposed();

                params._constantBufferFS._lightPos = math::Vector4D(25.0f, 0.0f, 5.0f, 1.0f);

                //ps
                params._texture = model->m_Texture.get();
                params._sampler = model->m_Sampler.get();
            });

        m_Render->process(*m_CmdList, model->m_Props);
    }

    m_Device->submit(m_CmdList);

    m_Swapchain->endFrame();
    m_Swapchain->presentFrame();
}

void Scene::Exit()
{
    m_Device->waitGPUCompletion(m_CmdList);

    for (auto& model : m_Models)
    {
        delete model->m_Model;
        delete model;
    }
    m_Models.clear();

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