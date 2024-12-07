#include "Scene.h"
#include "Scene/CameraArcballHandler.h"
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
    , m_Worker(2)

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
    m_Camera = new scene::CameraFPSHandler(new scene::Camera(math::Vector3D(0.0f, 0.0f, 0.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), { 0.0f, 0.0f, 0.0f });
    m_Camera->setPerspective(45.0f, m_Swapchain->getBackbufferSize(), 1.f, 50.f);
    m_Camera->setRotation(math::Vector3D(0.0f, -90.0f, 0.0f));
}

void Scene::Load()
{
    v3d::renderer::CmdListRender* CmdList = m_Device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    //TODO json scene loader

    //Geometry
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
            CmdList->uploadData(voyager->m_Texture.get(), image->getSize(), image->getBitmap());
        }

        {
            resource::ModelResource* modelRes = resource::ResourceManager::getInstance()->load<resource::ModelResource, resource::ModelFileLoader>("models/voyager/voyager.dae",
                resource::ModelFileLoader::ModelLoaderFlag::FlipYTextureCoord | resource::ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent | resource::ModelFileLoader::ModelLoaderFlag::SkipMaterial);

            scene::Model* model = scene::ModelHelper::createModel(m_Device, CmdList, modelRes);
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
    }

    //Pipeline
    {
#if VULKAN_ONLY
        const renderer::Shader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.vert");
        const renderer::Shader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.frag");
#else
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.hlsl", "main_vs");
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(m_Device, "shaders/mesh.hlsl", "main_ps");
#endif
        v3d::renderer::ShaderProgram* program = new renderer::ShaderProgram(m_Device, vertShader, fragShader);

        v3d::renderer::RenderTargetState* renderTarget = new renderer::RenderTargetState(m_Device, m_Swapchain->getBackbufferSize());
        renderTarget->setColorTexture(0, m_Swapchain->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
            },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
        });
#if defined(PLATFORM_ANDROID)
        m_depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment,
            renderer::Format::Format_D24_UNorm_S8_UInt, m_Swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#else
        v3d::renderer::Texture2D* depthAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment,
            renderer::Format::Format_D32_SFloat_S8_UInt, m_Swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1);
#endif
        renderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0.0f);

        v3d::renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(m_Device, m_Models.front()->m_InputAttrib, program, renderTarget, "TextureRender");
        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);
    }

    m_Device->submit(CmdList, true);
}

void Scene::Draw(f32 dt)
{
    //Frame
    m_Swapchain->beginFrame();

    m_Camera->update(dt);

    for (auto& [render, task, id] : m_RenderGroups)
    {
        render->setup(m_RenderTarget, *m_Camera);
        task->init([](Render* render, ModelsGroup* group) -> void
            {
                render->process(group->m_Pipeline, group->m_InputProps);
            }, render, m_ModelGroups[id]);

        m_Worker.executeTask(task, task::TaskPriority::Normal, task::TaskMask::WorkerThread);
    }

    for (auto& [render, task, id] : m_RenderGroups)
    {
        m_Worker.waitTask(task);
    }

    for (auto& [render, task, id] : m_RenderGroups)
    {
        m_Device->submit(render->_CmdList);
    }

    m_Swapchain->endFrame();
    m_Swapchain->presentFrame();
}

void Scene::Exit()
{
    //m_Device->waitGPUCompletion(m_CmdList);

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
        //scene->m_Camera->handlerMouseCallback(handler, event);
    }
}

void Scene::TouchCallback(Scene* scene, event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
    if (scene->m_Camera)
    {
        //scene->m_Camera->handlerTouchCallback(handler, event);
    }
}

} //namespace app