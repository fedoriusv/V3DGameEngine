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
#include "Resource/AssetSourceFileLoader.h"
#include "Stream/StreamManager.h"
#include "Resource/ShaderCompiler.h"

#define VULKAN_ONLY 0

namespace app
{

using namespace v3d;

const u32 k_workerThreadsCount = 4;

Scene::Scene(v3d::renderer::Device* device, renderer::Swapchain* swapchain) noexcept
    : m_Device(device)
    , m_Swapchain(swapchain)
    , m_Worker(k_workerThreadsCount)

    , m_CurrentState(States::StateInit)

    , m_Camera(nullptr)
    , m_Render(nullptr)
{
    resource::ResourceManager::createInstance();
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
    m_Camera = new scene::CameraFPSHandler(new scene::Camera(math::Vector3D(0.0f, 0.0f, -8.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), { 0.0f, 1.0f, -10.0f });
    m_Camera->setPerspective(45.0f, m_Swapchain->getBackbufferSize(), 0.01f, 256.f);

    m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    m_Sync = m_Device->createSyncPoint(m_CmdList);

    //Backbuffer
    {
        const renderer::VertexShader* vertexShader = nullptr;
        {
            const std::string vertexSource("\
                    struct VS_OUTPUT\n\
                    {\n\
                        float4 Pos : SV_Position;\n\
                        float2 UV  : TEXTURE;\n\
                    };\n\
                    \n\
                    VS_OUTPUT main(uint vertexID : SV_VertexID)\n\
                    {\n\
                        VS_OUTPUT output;\n\
                        output.UV = float2((vertexID << 1) & 2, vertexID & 2);\n\
                        output.Pos = float4(output.UV.x * 2.0f - 1.0f, -(output.UV.y * 2.0f - 1.0f), 0.0f, 1.0f);\n\
                        return output;\n\
                    }");
            const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

            resource::ShaderDecoder::ShaderPolicy vertexPolicy;
            vertexPolicy._type = renderer::ShaderType::Vertex;
            vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            vertexPolicy._content = renderer::ShaderContent::Source;
            vertexPolicy._entryPoint = "main";

            vertexShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(m_Device, "vertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(vertexStream);
        }

        const renderer::FragmentShader* fragmentShader = nullptr;
        {
            const std::string fragmentSource("\
                    struct VS_OUTPUT\n\
                    {\n\
                        float4 Pos : SV_Position;\n\
                        float2 UV  : TEXTURE;\n\
                    };\n\
                    [[vk::binding(0, 0)]] SamplerState colorSampler : register(s0, space0);\n\
                    [[vk::binding(1, 0)]] Texture2D colorTexture : register(t1, space0);\n\
                    \n\
                    float4 main(VS_OUTPUT input) : SV_TARGET0\n\
                    {\n\
                        float4 outFragColor = colorTexture.Sample(colorSampler, input.UV);\n\
                        return outFragColor;\n\
                    }");
            const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

            resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
            fragmentPolicy._type = renderer::ShaderType::Fragment;
            fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            fragmentPolicy._content = renderer::ShaderContent::Source;
            fragmentPolicy._entryPoint = "main";

            fragmentShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(m_Device, "fragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(fragmentStream);

            m_ProgramBackbuffer = new renderer::ShaderProgram(m_Device, vertexShader, fragmentShader);
        }

        m_renderTargetBackbuffer = new renderer::RenderTargetState(m_Device, m_Swapchain->getBackbufferSize());
        m_renderTargetBackbuffer->setColorTexture(0, m_Swapchain->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(1.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
            });

        m_PipelineBackbuffer = new renderer::GraphicsPipelineState(m_Device, renderer::VertexInputAttributeDesc(), m_ProgramBackbuffer, m_renderTargetBackbuffer, "Backbuffer");
        m_PipelineBackbuffer->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_PipelineBackbuffer->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_PipelineBackbuffer->setCullMode(renderer::CullMode::CullMode_Back);
        m_PipelineBackbuffer->setColorMask(renderer::ColorMask::ColorMask_All);
        m_PipelineBackbuffer->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        m_PipelineBackbuffer->setDepthWrite(false);
        m_PipelineBackbuffer->setDepthTest(false);

        m_samplerBackbuffer = new renderer::SamplerState(m_Device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    }
}

void Scene::Load()
{
    resource::ResourceManager::getInstance()->addPath("../../../../examples/multithreadeddraws/data/models/");
    resource::ResourceManager::getInstance()->addPath("../../../../examples/multithreadeddraws/data/shaders/");
    resource::ResourceManager::getInstance()->addPath("../../../../examples/multithreadeddraws/data/textures/");

    //TODO json scene loader
    //resource::Resource* material = resource::ResourceManager::getInstance()->load<resource::Resource, resource::AssetSourceFileLoader>("materials/texture.material.json");

    //Material
    v3d::renderer::ShaderProgram* program = nullptr;
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(m_Device, "lit_texture.hlsl", "main_vs");
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(m_Device, "lit_texture.hlsl", "main_ps");
        program = new renderer::ShaderProgram(m_Device, vertShader, fragShader);
    }

    resource::Bitmap* image = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("basetex.jpg");

    renderer::SamplerState* sampler = new renderer::SamplerState(m_Device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    sampler->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    renderer::Texture2D* texture = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
        image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), image->getMipmapsCount());

    m_CmdList->uploadData(texture, image->getSize(), image->getBitmap());


    //Geometry
    scene::Model* model = nullptr;
    v3d::renderer::GraphicsPipelineState* pipeline = nullptr;
    {
        resource::ModelResource* modelRes = resource::ResourceManager::getInstance()->load<resource::ModelResource, resource::ModelFileLoader>("cube.dae", resource::ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent);
        model = scene::ModelHelper::createModel(m_Device, m_CmdList, modelRes);
        resource::ResourceManager::getInstance()->remove(modelRes);
    }

    //Pipeline
    {
        const v3d::renderer::VertexInputAttributeDesc& attrib = model->m_geometry[0]._LODs[0]->getVertexAttribDesc(); //load from json
        m_RenderTarget = new renderer::RenderTargetState(m_Device, m_Swapchain->getBackbufferSize());
        v3d::renderer::Texture2D* colorAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
            renderer::Format::Format_R8G8B8A8_UNorm, m_Swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
        m_RenderTarget->setColorTexture(0, colorAttachment,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
            });

        v3d::renderer::Texture2D* depthAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Write,
            renderer::Format::Format_D32_SFloat_S8_UInt, m_Swapchain->getBackbufferSize(), renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
        m_RenderTarget->setDepthStencilTexture(depthAttachment, 
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
            },
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0
            },
            {
                 renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
            });

        pipeline = new renderer::GraphicsPipelineState(m_Device, attrib, program, m_RenderTarget, "TextureRender");
        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);
    }

    //Scene
    {
        ModelData* data = new ModelData();
        data->_Pipeline = pipeline;
        for (auto& geom : model->m_geometry)
        {
            scene::Mesh* mesh = geom._LODs[0]; //first lod only
            if (mesh->m_indexBuffer)
            {
                v3d::renderer::GeometryBufferDesc desc(mesh->m_indexBuffer, 0, mesh->m_vertexBuffer[0], 0, mesh->getVertexAttribDesc()._inputBindings[0]._stride, 0);
                app::DrawProperties prop{ 0, mesh->m_indexBuffer->getIndicesCount(), 0, 1, true };
                data->_Props.emplace_back(std::move(desc), prop);
                data->_InputAttrib = mesh->getVertexAttribDesc();
            }
            else
            {
                v3d::renderer::GeometryBufferDesc desc(mesh->m_vertexBuffer[0], 0, mesh->getVertexAttribDesc()._inputBindings[0]._stride);
                app::DrawProperties prop{ 0, mesh->m_vertexBuffer[0]->getVerticesCount(), 0, 1, false };
                data->_Props.emplace_back(std::move(desc), prop);
                data->_InputAttrib = mesh->getVertexAttribDesc();
            }
        }
        m_Models.emplace(data);
    }
    m_Pipelines.push_back(pipeline);

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(-10, 10); // define the range

    const u32 instanceCountPerModel = 40; //load from json
    for (ModelData* data : m_Models)
    {
        for (u32 i = 0; i < instanceCountPerModel; ++i)
        {
            ParameterData params;
            params._Texture = texture;
            params._Sampler = sampler;
            params._Transform.makeIdentity();
            params._Transform.setTranslation(math::Vector3D(distr(gen), distr(gen), distr(gen)));
            params._Transform.setScale({ 100.0f, 100.0f, 100.0f });

            auto pipelineIter = std::find(m_Pipelines.cbegin(), m_Pipelines.cend(), pipeline);
            ASSERT(pipelineIter != m_Pipelines.cend(), "not found");
            u32 id = std::distance(m_Pipelines.cbegin(), pipelineIter);

            ModelsGroup models{ data->_Props, params, id };
            m_ModelInstances.push_back(models);
        }
    }

    //Render Worker for 0 material
    {
        u32 materialIndex = 0; //load from json
        m_RenderGroups.resize(k_workerThreadsCount);
        u32 perGroup = m_ModelInstances.size() / m_RenderGroups.size();
        u32 offsetGroup = 0;
        u32 rangeGroup = perGroup;
        for (auto& group : m_RenderGroups)
        {
            TextureRenderWorker* worker = new TextureRenderWorker;
            worker->_CmdList = m_Device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
            group = { worker, new task::Task, materialIndex, offsetGroup, rangeGroup };
            offsetGroup += rangeGroup;
        }
    }

    m_Device->submit(m_CmdList, true);
}

void Scene::Draw(f32 dt)
{
    //Frame
    m_Swapchain->beginFrame();

    m_Camera->update(dt);

    m_CmdList->transition(renderer::TextureView(m_RenderTarget->getColorTexture<v3d::renderer::Texture2D>(0), 0, 1, 0, 1), renderer::TransitionOp::TransitionOp_ColorAttachment);
    m_CmdList->clear(m_RenderTarget->getColorTexture<v3d::renderer::Texture2D>(0), math::Vector4D(0.0f));
    m_CmdList->clear(m_RenderTarget->getDepthStencilTexture<v3d::renderer::Texture2D>(), 0.0f, 0);
    m_Device->submit(m_CmdList);

    for (auto& [render, task, pipelineID, offset, range] : m_RenderGroups)
    {
        render->setup(m_RenderTarget, *m_Camera);
        task->init([this](Render* render, v3d::renderer::GraphicsPipelineState* pipeline,u32 groupStart, u32 countGroups) -> void
            {
                render->_CmdList->beginRenderTarget(*render->_RenderTarget);
                render->_CmdList->setViewport(math::Rect32(0, 0, render->_RenderTarget->getRenderArea().m_width, render->_RenderTarget->getRenderArea().m_height));
                render->_CmdList->setScissor(math::Rect32(0, 0, render->_RenderTarget->getRenderArea().m_width, render->_RenderTarget->getRenderArea().m_height));

                for (u32 groupIndex = 0; groupIndex < countGroups; ++groupIndex)
                {
                    ModelsGroup& group = m_ModelInstances[groupStart + groupIndex];
                    render->updateParameters([group](Render* render) -> void
                        {
                            TextureRenderWorker* tRender = static_cast<TextureRenderWorker*>(render);

                            tRender->_LightParams._constantBuffer._lightPos = math::Vector4D(25.0f, 0.0f, 5.0f, 1.0f);
                            tRender->_LightParams._constantBuffer._color = math::Vector4D(1.0f);

                            tRender->_TextureParams._constantBufferVS._modelMatrix = group._Parameters._Transform;
                            tRender->_TextureParams._constantBufferVS._normalMatrix = group._Parameters._Transform;
                            tRender->_TextureParams._constantBufferVS._normalMatrix.makeTransposed();
                            tRender->_TextureParams._texture = group._Parameters._Texture;
                            tRender->_TextureParams._sampler = group._Parameters._Sampler;
                        });
                    render->process(pipeline, group._InputProps);
                }
            }, render, m_Pipelines[pipelineID], offset, range);

        m_Worker.executeTask(task, task::TaskPriority::Normal, task::TaskMask::WorkerThread);
    }
     
    m_Worker.mainThreadLoop();
    for (auto& [render, task, pipelineID, offset, range] : m_RenderGroups)
    {
        m_Worker.waitTask(task);
    }

    for (auto& [render, task, pipelineID, offset, range] : m_RenderGroups)
    {
        m_Device->submit(render->_CmdList);
    }

    //Backbuffer
    {
        m_CmdList->beginRenderTarget(*m_renderTargetBackbuffer);
        m_CmdList->setViewport(math::Rect32(0, 0, m_renderTargetBackbuffer->getRenderArea().m_width, m_renderTargetBackbuffer->getRenderArea().m_height));
        m_CmdList->setScissor(math::Rect32(0, 0, m_renderTargetBackbuffer->getRenderArea().m_width, m_renderTargetBackbuffer->getRenderArea().m_height));
        m_CmdList->setPipelineState(*m_PipelineBackbuffer);

        renderer::Descriptor colorSampler(m_samplerBackbuffer, 0);
        renderer::Descriptor colorTexture(m_RenderTarget->getColorTexture<v3d::renderer::Texture2D>(0), 1);
        m_CmdList->bindDescriptorSet(0, { colorSampler, colorTexture });
        m_CmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);

        m_CmdList->endRenderTarget();

        m_Device->submit(m_CmdList, m_Swapchain->getSyncPoint());
    }

    m_Swapchain->endFrame();
    m_Swapchain->presentFrame(m_Sync);
}

void Scene::Exit()
{
    m_Device->waitGPUCompletion(m_CmdList);
    m_Device->destroySyncPoint(m_CmdList, m_Sync);
    m_Device->destroyCommandList(m_CmdList);

    for (auto& [render, task, pipelineID, offset, range] : m_RenderGroups)
    {
        m_Device->destroyCommandList(render->_CmdList);
        delete render;
        delete task;
    }
    m_RenderGroups.clear();

    //for (auto& model : m_Models)
    //{
    //    delete model->m_Model;
    //    delete model;
    //}
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
        scene->m_Camera->rotateHandlerCallback(handler, event, false);
    }
}

void Scene::KeyboardCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::KeyboardInputEvent* event)
{
    if (scene->m_Camera)
    {
        scene->m_Camera->moveHandlerCallback(handler, event);
    }
}

} //namespace app