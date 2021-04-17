#include "Render.h"

#include "Renderer/Shader.h"
#include "Resource/Image.h"

#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ResourceLoaderManager.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"

#include "Scene/Model.h"
#include "Scene/ModelHelper.h"

using namespace v3d;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DrawTexturedCube : public DrawPolicy
{
public:

    explicit DrawTexturedCube(SceneRenderer* scene) noexcept
        : m_Scene(scene)

        , m_Program(nullptr)
        , m_Pipeline(nullptr)

        , m_Sampler(nullptr)
        , m_Texture(nullptr)
        , m_Geometry(nullptr)
    {
    }

    ~DrawTexturedCube()
    {
        delete m_Geometry;
        delete m_Texture;
        delete m_Sampler;

        delete m_Pipeline;
        delete m_Program;
    }

    void Init(v3d::renderer::CommandList* commandList, v3d::renderer::RenderTargetState* target) override
    {
        resource::ResourceLoaderManager::getInstance()->addPath("examples/rendertarget/data/");

        m_Sampler = commandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("basetex.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
        ASSERT(image, "not found");
        m_Texture = commandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write,
            image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), image->getMipMapsCount(), image->getRawData(), "UnlitTexture");

        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList->getContext(), "texture.hlsl",
            {
                {"main_VS", renderer::ShaderType_Vertex },
                {"main_FS", renderer::ShaderType_Fragment }
            }, {}, resource::ShaderSource_UseDXCompiler);

        v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("cube.dae");
        m_Geometry = v3d::scene::ModelHelper::createModelHelper(commandList, { cube });


        m_Program = commandList->createObject<renderer::ShaderProgram>(shaders);
        m_Pipeline = commandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Program, target);
        m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Pipeline->setDepthWrite(true);
        m_Pipeline->setDepthTest(true);

        ///
        ////Compute
        //if (m_ComputeDownsampling)
        //{
        //    const renderer::Shader* shader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "downsampling.comp");
        //    m_DownsampleProgram = m_CommandList->createObject<renderer::ShaderProgram>(shader);
        //    m_DownsamplePipeline = m_CommandList->createObject<renderer::ComputePipelineState>(m_DownsampleProgram);

        //    m_testTexture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write | renderer::TextureUsage::TextureUsage_Storage,
        //        m_ColorAttachment->getFormat(), core::Dimension2D(m_ColorAttachment->getDimension().width, m_ColorAttachment->getDimension().height), 1, nullptr, "TestTexture");
        //}
    }

    void Bind(v3d::renderer::CommandList* commandList) override
    {
        commandList->setPipelineState(m_Pipeline);

        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D viewMatrix;
            core::Matrix4D modelMatrix;
        } ubo;

        ubo.projectionMatrix = m_Scene->getActiveCamera()->getProjectionMatrix();
        ubo.viewMatrix = m_Scene->getActiveCamera()->getViewMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setScale({ 100.0f, 100.0f, 100.0f });

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);

        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "colorSampler" }, m_Sampler);
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "colorTexture" }, m_Texture);
    }
    
    void Draw(v3d::renderer::CommandList* commandList) override
    {
        m_Geometry->draw(commandList);
    }

public:

    SceneRenderer* m_Scene;

    v3d::renderer::GraphicsPipelineState* m_Pipeline;
    v3d::renderer::ShaderProgram* m_Program;

    v3d::renderer::SamplerState* m_Sampler;
    v3d::renderer::Texture2D* m_Texture;
    v3d::scene::ModelHelper* m_Geometry;


    ////compute
    //v3d::renderer::ComputePipelineState* m_DownsamplePipeline;
    //v3d::renderer::ShaderProgram* m_DownsampleProgram;
    //bool m_ComputeDownsampling = false;

    //v3d::renderer::Texture2D* m_testTexture;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BasePassRender : public BaseRender
{
public:

    BasePassRender(const core::Dimension2D& viewport) noexcept
        : m_RenderTarget(nullptr)
        , m_ColorAttachment(nullptr)
        , m_DepthAttachment(nullptr)

        , m_Viewport(viewport)
    {
    }

    ~BasePassRender()
    {
        delete m_RenderTarget;
        delete m_ColorAttachment;
        delete m_DepthAttachment;
    }

    void Init(v3d::renderer::CommandList* commandList) override
    {
        m_RenderTarget = commandList->createObject<renderer::RenderTargetState>(m_Viewport, 0, "RenderTarget");

        m_ColorAttachment = commandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_GenerateMipmaps | renderer::TextureUsage::TextureUsage_Storage,
            renderer::Format::Format_R8G8B8A8_UNorm, m_Viewport, renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
        m_RenderTarget->setColorTexture(0, m_ColorAttachment,
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
            });

#if defined(PLATFORM_ANDROID)
        m_DepthAttachment = commandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D24_UNorm_S8_UInt, m_Viewport, renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
#else

        m_DepthAttachment = commandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_Viewport, renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
#endif
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
    
    void Render(v3d::renderer::CommandList* commandList, DrawPolicy* draw) override
    {
        commandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        commandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        commandList->setRenderTarget(m_RenderTarget);

        draw->Bind(commandList);
        draw->Draw(commandList);

        //if (m_ComputeDownsampling) //compute
        //{
        //    m_CommandList->transition<renderer::Texture2D>(m_ColorAttachment, renderer::TransitionOp::TransitionOp_GeneralCompute, 0, 0);
        //    m_CommandList->transition<renderer::Texture2D>(m_testTexture, renderer::TransitionOp::TransitionOp_GeneralCompute, 0, 0);

        //    m_CommandList->setPipelineState(m_DownsamplePipeline);

        //    m_DownsampleProgram->bindStorageImage<renderer::ShaderType::ShaderType_Compute, renderer::Texture2D>({ "inputImage" }, m_ColorAttachment, 0, 0);
        //    m_DownsampleProgram->bindStorageImage<renderer::ShaderType::ShaderType_Compute, renderer::Texture2D>({ "resultImage" }, m_testTexture, 0, 0);

        //    m_CommandList->dispatchCompute({ m_ColorAttachment->getDimension().width / 4, m_ColorAttachment->getDimension().height / 4, 1 });

        //    m_CommandList->transition<renderer::Texture2D>(m_testTexture, renderer::TransitionOp::TransitionOp_ShaderRead, 0, 0);
        //}
        //else
        //{
        //    m_CommandList->generateMipmaps(m_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead);
        //}
    }

public:

    v3d::renderer::RenderTargetState* m_RenderTarget;
    v3d::renderer::Texture2D* m_ColorAttachment;
    v3d::renderer::Texture2D* m_DepthAttachment;

    core::Dimension2D m_Viewport;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OffscreenRender : public BaseRender
{
public:

    explicit OffscreenRender(v3d::renderer::Texture2D* input) noexcept
        : m_OffscreenRenderTarget(nullptr)
        , m_OffscreenProgram(nullptr)
        , m_OffscreenPipeline(nullptr)
        , m_Sampler(nullptr)
        , m_OffscreenAttachment(input)
    {
    }

    ~OffscreenRender()
    {
        delete m_OffscreenPipeline;
        delete m_OffscreenProgram;

        delete m_OffscreenRenderTarget;
        m_OffscreenAttachment = nullptr;

        delete m_Sampler;

    }

    void Init(v3d::renderer::CommandList* commandList) override
    {
        resource::ResourceLoaderManager::getInstance()->addPath("examples/rendertarget/data/");

        const u32 k_mipLevel = 3;
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList->getContext(), "offscreen.hlsl",
            {
                {"main_VS", renderer::ShaderType_Vertex },
                {"main_FS", renderer::ShaderType_Fragment }

            },
            {
                { "MIP_LEVEL", std::to_string(k_mipLevel) }
            }, resource::ShaderSource_UseDXCompiler);

        m_OffscreenRenderTarget = commandList->createObject<renderer::RenderTargetState>(commandList->getBackbuffer()->getDimension(), 0, "OffscreenTarget");
        m_OffscreenRenderTarget->setColorTexture(0, commandList->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
            });

        m_OffscreenProgram = commandList->createObject<renderer::ShaderProgram>(shaders);
        m_OffscreenPipeline = commandList->createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_OffscreenProgram, m_OffscreenRenderTarget);
        m_OffscreenPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_OffscreenPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_OffscreenPipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_OffscreenPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_OffscreenPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        m_OffscreenPipeline->setDepthWrite(false);
        m_OffscreenPipeline->setDepthTest(false);

        m_Sampler = commandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
    }

    void Render(v3d::renderer::CommandList* commandList, DrawPolicy* draw) override
    {
        commandList->setViewport(core::Rect32(0, 0, m_OffscreenRenderTarget->getDimension().width, m_OffscreenRenderTarget->getDimension().height));
        commandList->setScissor(core::Rect32(0, 0, m_OffscreenRenderTarget->getDimension().width, m_OffscreenRenderTarget->getDimension().height));
        commandList->setRenderTarget(m_OffscreenRenderTarget);
        commandList->setPipelineState(m_OffscreenPipeline);

        m_OffscreenProgram->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "colorSampler" }, m_Sampler);
        m_OffscreenProgram->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "colorTexture" }, m_OffscreenAttachment, 0, 0);

        commandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
    }

public:

    v3d::renderer::RenderTargetState* m_OffscreenRenderTarget;
    v3d::renderer::ShaderProgram* m_OffscreenProgram;
    v3d::renderer::GraphicsPipelineState* m_OffscreenPipeline;

    v3d::renderer::SamplerState* m_Sampler;
    v3d::renderer::Texture2D* m_OffscreenAttachment;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneRenderer::SceneRenderer(v3d::renderer::CommandList& commandList) noexcept
    : m_CommandList(commandList)
    , m_Camera(nullptr)

    , m_BasePass(nullptr)
    , m_OffcreenPass(nullptr)

    , m_Draws(nullptr)
{
}

SceneRenderer::~SceneRenderer()
{
    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = nullptr;
    }

    delete m_BasePass;
    delete m_OffcreenPass;

    delete m_Draws;
}

void SceneRenderer::Prepare(const core::Dimension2D& size)
{
    m_Camera = new v3d::scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), 5.0f, 4.0f, 20.0f);
    m_Camera->setPerspective(45.0f, size, 0.1f, 256.f);


    BasePassRender* mainRender = new BasePassRender(m_CommandList.getBackbuffer()->getDimension());
    mainRender->Init(&m_CommandList);
    m_BasePass = mainRender;

    OffscreenRender* offscreen = new OffscreenRender(mainRender->m_ColorAttachment);
    offscreen->Init(&m_CommandList);
    m_OffcreenPass = offscreen;

    DrawTexturedCube* cube = new DrawTexturedCube(this);
    cube->Init(&m_CommandList, mainRender->m_RenderTarget);
    m_Draws = cube;

    m_CommandList.submitCommands();
    m_CommandList.flushCommands();
}

void SceneRenderer::Render(f32 dt)
{
    m_Camera->update(0);

    m_CommandList.beginFrame();

    m_BasePass->Render(&m_CommandList, m_Draws);
    m_CommandList.submitCommands(true);
    m_OffcreenPass->Render(&m_CommandList, nullptr);
    m_CommandList.submitCommands(true);
    m_CommandList.endFrame();
    m_CommandList.presentFrame();

    m_CommandList.flushCommands();
}

scene::CameraArcballHelper* SceneRenderer::getActiveCamera() const
{
    return m_Camera;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
