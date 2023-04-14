#include "Scene.h"

#include "Event/InputEventMouse.h"
#include "Event/InputEventTouch.h"
#include "Event/InputEventKeyboard.h"
#include "Utils/Logger.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Bitmap.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/StreamBuffer.h"
#include "Renderer/SamplerState.h"
#include "Renderer/Texture.h"

#include "Scene/Model.h"

#include "BasePass.h"
#include "OffscreenPass.h"

Scene::Scene(renderer::CommandList& cmdList, const math::Dimension2D& size) noexcept
    : m_CommandList(cmdList)
{
    m_FPSCameraHelper = new scene::CameraFPSHelper(new scene::Camera(math::Vector3D(0.0f, 0.0f, 0.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), math::Vector3D(0.0f, 0.0f, -90.0f));
    m_FPSCameraHelper->setPerspective(60.0f, size, 0.001f, 256.f);

    m_BasePassDraw = new BasePassDraw();
    m_BasePassDraw->Init(m_CommandList, size);

    m_SwapchainPassDraw = new OffscreenPassDraw(true);
    m_SwapchainPassDraw->Init(m_CommandList, size);

    resource::ResourceLoaderManager::getLazyInstance()->addPath("examples/querytest/resources/");
    LoadScene();

    m_CommandList.submitCommands(true);
    m_CommandList.flushCommands();
}

void Scene::LoadScene()
{
    {
        scene::Model* mesh = resource::ResourceLoaderManager::getLazyInstance()->load<scene::Model, resource::ModelFileLoader>("cube.dae");
        ASSERT(mesh, "not found");
        scene::Mesh* meshGeometry = mesh->getMeshByIndex(0);

        {
            renderer::QueryTimestampRequest* timeStampQuery = m_CommandList.createObject<renderer::QueryTimestampRequest>([this](const std::vector<u64>& timestamp, const std::vector<std::string>& tags)-> void
                {
                    for (u32 i = 0; i < m_Measurements._QueryTimings.size(); ++i)
                    {
                        m_Measurements._QueryTimings[i]._BeginTime = timestamp[i * 2];
                        m_Measurements._QueryTimings[i]._EndTime = timestamp[i * 2 + 1];
                    }
                }, m_Measurements._QueryTimings.size() * 2);
            m_Resources.push_back(timeStampQuery);


            BasePassDraw::RenderPolicy* render = new BasePassDraw::TexturedRender(meshGeometry->getInputAttributeDesc());
            render->Init(m_CommandList, m_BasePassDraw->GetRenderTarget());
            m_DrawList._Render = render;
            m_DrawList._TimeStampQuery = timeStampQuery;
            m_DrawList._Profiler = &m_SceneProfiler;
            m_Renderers.push_back(render);

            OffscreenPassDraw::OffsceenRender* offscreen = new  OffscreenPassDraw::OffsceenRender();
            offscreen->Init(m_CommandList, m_SwapchainPassDraw->GetRenderTarget());
            m_OffscreenDraw._Render = offscreen;
            m_OffscreenDraw._TimeStampQuery = timeStampQuery;
            m_OffscreenDraw._Profiler = &m_SceneProfiler;
            m_Renderers.push_back(offscreen);
        }

        const u32 k_meshCount = 1000;
        for (u32 i = 0; i < k_meshCount; ++i)
        {
            renderer::VertexStreamBuffer* vertexBuffer = m_CommandList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared,
                meshGeometry->getVertexSize(), meshGeometry->getVertexData(), "VertexBufferBox");
            m_Resources.push_back(vertexBuffer);

            renderer::IndexStreamBuffer* indexBuffer = m_CommandList.createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared,
                renderer::StreamIndexBufferType::IndexType_32, meshGeometry->getIndexCount(), meshGeometry->getIndexData(), "IndexBufferBox");
            m_Resources.push_back(indexBuffer);

            BaseDraw::MeshInfo* meshdata = new BaseDraw::MeshInfo
            {
                renderer::StreamBufferDescription(indexBuffer, 0, vertexBuffer, 0, 0),
                mesh->getMeshByIndex(0)->getInputAttributeDesc(),
                { 0, mesh->getMeshByIndex(0)->getIndexCount(), 0, 1, true },
            };
            m_DrawList._DrawState.push_back(std::make_tuple(new BaseDraw::ProgramParams(), meshdata));
        }
    }

    {
        renderer::SamplerState* sampler = m_CommandList.createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_2x);
        sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
        m_Resources.push_back(sampler);

        resource::Bitmap* image0 = resource::ResourceLoaderManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("box.jpg", resource::ImageFileLoader::ImageLoaderFlag::GenerateMipmaps);
        ASSERT(image0, "not found");
        renderer::Texture2D* texture0 = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image0->getFormat(),
            math::Dimension2D(image0->getDimension().m_width, image0->getDimension().m_height), image0->getMipMapsCount(), image0->getBitmap(), "Box");
        m_Resources.push_back(texture0);

        resource::Bitmap* image1 = resource::ResourceLoaderManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("basetex.jpg", resource::ImageFileLoader::ImageLoaderFlag::GenerateMipmaps);
        ASSERT(image1, "not found");
        renderer::Texture2D* texture1 = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image1->getFormat(),
            math::Dimension2D(image1->getDimension().m_width, image1->getDimension().m_height), image1->getMipMapsCount(), image1->getBitmap(), "Tex");
        m_Resources.push_back(texture1);

        auto randomFloat = [](f32 min, f32 max) -> f32
        {
            f32 r = (f32)rand() / (f32)RAND_MAX;
            return min + r * (max - min);
        };
        
        for (auto& state : m_DrawList._DrawState)
        {
            math::Matrix4D transform;

            f32 locationX = randomFloat(-30.f, 30.f);
            f32 locationY = randomFloat(-30.f, 30.f);
            f32 locationZ = randomFloat(-30.f, 30.f);
            transform.setTranslation({ locationX, locationY, locationZ });

            f32 scale = randomFloat(1.f, 3.f) * 100.f;
            transform.setScale({ scale, scale, scale });

            BaseDraw::ProgramParams* params = std::get<0>(state);
            params->_ConstantBuffer.modelMatrix = transform;
            params->_Sampler = sampler;
            params->_Texture = ((u32)randomFloat(1.f, 10.f) % 2) ? texture0 : texture1;
        }

        m_OffscreenDraw._DrawState.push_back(std::make_tuple(new BaseDraw::ProgramParams
            {
                BaseDraw::ProgramParams::UBO(),
                sampler,
                m_BasePassDraw->GetRenderTarget()->getColorTexture<renderer::Texture2D>(0)
            }
        , nullptr));
    }
}

void Scene::UpdateScene()
{
    m_SceneProfiler.start("SceneLoop.SceneUpdate");

    for (auto& state : m_DrawList._DrawState)
    {
        BaseDraw::ProgramParams* params = std::get<0>(state);

        params->_ConstantBuffer.projectionMatrix = m_FPSCameraHelper->getProjectionMatrix();
        params->_ConstantBuffer.modelMatrix; //TODO set offset
        params->_ConstantBuffer.modelMatrix.getInverse(params->_ConstantBuffer.normalMatrix);
        params->_ConstantBuffer.normalMatrix.makeTransposed();
        params->_ConstantBuffer.viewMatrix = m_FPSCameraHelper->getViewMatrix();
    }
    m_SceneProfiler.stop("SceneLoop.SceneUpdate");

}

void Scene::FreeScene()
{
    for (auto& state : m_DrawList._DrawState)
    {
        BaseDraw::ProgramParams* params = std::get<0>(state);
        delete params;

        BaseDraw::MeshInfo* meshInfo = std::get<1>(state);
        delete meshInfo;
    }
    m_DrawList._DrawState.clear();
    m_DrawList._Render = nullptr;

    for (auto& state : m_OffscreenDraw._DrawState)
    {
        delete std::get<0>(state);
    }
    m_OffscreenDraw._DrawState.clear();
    m_OffscreenDraw._Render = nullptr;

    for (BasePassDraw::RenderPolicy* render : m_Renderers)
    {
        delete render;
    }
    m_Renderers.clear();

    for (Object* object : m_Resources)
    {
        delete object;
    }
    m_Resources.clear();
}

Scene::~Scene()
{
    FreeScene();

    delete m_BasePassDraw;
    delete m_SwapchainPassDraw;

    delete m_FPSCameraHelper;

    resource::ResourceLoaderManager::getInstance()->clear();
    resource::ResourceLoaderManager::getInstance()->freeInstance();
}

void Scene::Run(f32 dt)
{
    m_SceneProfiler.update(dt);
    m_SceneProfiler.start("SceneLoop");

    m_FPSCameraHelper->update(dt);
    UpdateScene();

    m_CommandList.beginFrame();

    m_BasePassDraw->QueryTimeStamp(m_DrawList._TimeStampQuery, 0, "GPUframe start");

    m_BasePassDraw->Draw(m_CommandList, m_DrawList);
    m_SwapchainPassDraw->Draw(m_CommandList, m_OffscreenDraw);

    m_BasePassDraw->QueryTimeStamp(m_DrawList._TimeStampQuery, 1, "GPUframe end");

    m_CommandList.endFrame();
    m_CommandList.submitCommands();
    m_CommandList.presentFrame();

    m_SceneProfiler.start("SceneLoop.Flush");
    m_CommandList.flushCommands();
    m_SceneProfiler.stop("SceneLoop.Flush");

    static f32 k_sec = 0;
    if (k_sec > 1.0f)
    {
        m_Measurements._Draws = m_DrawList._DrawedLastFrame;
        m_Measurements.Print();
        k_sec = 0;
    }
    k_sec += dt;

    m_SceneProfiler.stop("SceneLoop");
}

void Scene::mouseHandle(const event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    m_FPSCameraHelper->rotateHandlerCallback(handler, event, false);
}

void Scene::touchHandle(const event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
}

void Scene::keyboardHandle(const event::InputEventHandler* handler, const event::KeyboardInputEvent* event)
{
    m_FPSCameraHelper->moveHandlerCallback(handler, event);
}

void Scene::Measurements::Print()
{
    LOG_WARNING("GPU Frametime: %.3f ms", (f32)(_QueryTimings[0]._EndTime - _QueryTimings[0]._BeginTime) / 1'000'000.f);
    LOG_WARNING("GPU Frametime OcclusionTest: %.3f ms", (f32)(_QueryTimings[3]._EndTime - _QueryTimings[3]._BeginTime) / 1'000'000.f);
    LOG_WARNING("GPU Frametime BasePass: %.3f ms. DrawCalls: %d", (f32)(_QueryTimings[1]._EndTime - _QueryTimings[1]._BeginTime) / 1'000'000.f, _Draws);
    LOG_WARNING("GPU Frametime Offscreen: %.3f ms", (f32)(_QueryTimings[2]._EndTime - _QueryTimings[2]._BeginTime) / 1'000'000.f);
}
