#include "Scene.h"

#include "Event/InputEventMouse.h"
#include "Event/InputEventTouch.h"
#include "Event/InputEventKeyboard.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ModelFileLoader.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"

#include "Scene/Model.h"

class MeshPolicy
{
public:

    void Init()
    {

    }

    void UpdateShader()
    {

    }

    void Draw()
    {

    }
};

Scene::Scene(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept
    : m_CommandList(cmdList)
{
    m_FPSCameraHelper = new scene::CameraFPSHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, -8.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), core::Vector3D(0.0f, 1.0f, -4.0f));
    m_FPSCameraHelper->setPerspective(45.0f, size, 0.01f, 256.f);

    {
        m_ColorAttachment = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8G8B8A8_UNorm,
            size, renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
        m_DepthAttachment = m_CommandList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt,
            size, renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");

        m_RenderTarget = m_CommandList.createObject<renderer::RenderTargetState>(size, 0, "RenderTarget");
        m_RenderTarget->setColorTexture(0, m_ColorAttachment,
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
            });
        m_RenderTarget->setDepthStencilTexture(m_DepthAttachment,
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f
            },
            {
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
            });
    }

    class TexturedRender : public Scene::BaseRender
    {
    public:

        TexturedRender() = default;
        ~TexturedRender() = default;

        void Init(renderer::CommandList& commandList, const renderer::VertexInputAttribDescription& desc, const renderer::RenderTargetState* renderTaget) override
        {
            std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "texture.hlsl",
                {
                    {"main_VS", renderer::ShaderType::Vertex },
                    {"main_FS", renderer::ShaderType::Fragment }
                }, {}, resource::ShaderSource_UseDXCompiler);

            m_Program = commandList.createObject<renderer::ShaderProgram>(shaders);
            m_Pipeline = commandList.createObject<renderer::GraphicsPipelineState>(desc, m_Program, renderTaget);
            m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
            m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
            m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
            m_Pipeline->setDepthWrite(true);
            m_Pipeline->setDepthTest(true);
        };

        void Render(renderer::CommandList& commandList) override
        {
            commandList.setPipelineState(m_Pipeline);

            //UpdateParams(m_Program);
            //commandList.draw();
        }

        void Destroy(renderer::CommandList& commandList) override
        {
            delete m_Pipeline;
            delete m_Program;
        }

        void UpdateParams(renderer::ShaderProgram*)
        {
            //struct UBO
            //{
            //    core::Matrix4D projectionMatrix;
            //    core::Matrix4D viewMatrix;
            //    core::Matrix4D modelMatrix;
            //} ubo;

            //ubo.projectionMatrix = m_Scene->getActiveCamera()->getProjectionMatrix();
            //ubo.viewMatrix = m_Scene->getActiveCamera()->getViewMatrix();
            //ubo.modelMatrix.makeIdentity();
            //ubo.modelMatrix.setScale({ 100.0f, 100.0f, 100.0f });

            //m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);

            //m_Program->bindSampler<renderer::ShaderType::Fragment>({ "colorSampler" }, m_Sampler);
            //m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "colorTexture" }, m_Texture);
        }

        renderer::GraphicsPipelineState* m_Pipeline;
        renderer::ShaderProgram* m_Program;

    };
    m_TexturedRender = new TexturedRender();

    scene::Model* mesh = nullptr;
    {
        mesh = resource::ResourceLoaderManager::getInstance()->load<scene::Model, resource::ModelFileLoader>("cube.dae");

        renderer::VertexStreamBuffer* vertexBuffer = m_CommandList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared,
            mesh->getMeshByIndex(0)->getVertexSize(), mesh->getMeshByIndex(0)->getVertexData());

        renderer::IndexStreamBuffer* indexBuffer = m_CommandList.createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared,
            renderer::StreamIndexBufferType::IndexType_32, mesh->getMeshByIndex(0)->getIndexCount(), mesh->getMeshByIndex(0)->getIndexData());
    }

    m_TexturedRender->Init(m_CommandList, mesh->getMeshByIndex(0)->getVertexInputAttribDesc(), m_RenderTarget);

    m_CommandList.submitCommands(true);
    m_CommandList.flushCommands();
}

Scene::~Scene()
{
    resource::ResourceLoaderManager::getInstance()->clear();
    resource::ResourceLoaderManager::getInstance()->freeInstance();
}

void Scene::Run(f32 dt)
{
    m_FPSCameraHelper->update(dt);

    {
        m_CommandList.beginFrame();

        m_TexturedRender->Render(m_CommandList);

        m_CommandList.endFrame();
        m_CommandList.presentFrame();

        m_CommandList.flushCommands();
    }
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
}
