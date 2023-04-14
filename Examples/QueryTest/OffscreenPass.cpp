#include "OffscreenPass.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

OffscreenPassDraw::OffsceenRender::OffsceenRender() noexcept
    : m_OffscreenPipeline(nullptr)
    , m_OffscreenProgram(nullptr)
{
}

OffscreenPassDraw::OffsceenRender::~OffsceenRender()
{
    delete m_OffscreenPipeline;
    delete m_OffscreenProgram;
}

void OffscreenPassDraw::OffsceenRender::Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget)
{
    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShaders<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "offscreen.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_FS", renderer::ShaderType::Fragment }
        });

    m_OffscreenProgram = commandList.createObject<renderer::ShaderProgram>(shaders);
    m_OffscreenPipeline = commandList.createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttributeDescription(), m_OffscreenProgram, renderTaget, "OffscreenPipeline");
    m_OffscreenPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_OffscreenPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_OffscreenPipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_OffscreenPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_OffscreenPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_OffscreenPipeline->setDepthWrite(false);
    m_OffscreenPipeline->setDepthTest(false);
}

void OffscreenPassDraw::OffsceenRender::Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData)
{
    commandList.setPipelineState(m_OffscreenPipeline);

    m_OffscreenProgram->bindSampler<renderer::ShaderType::Fragment>({ "colorSampler" }, params._Sampler);
    m_OffscreenProgram->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>({ "colorTexture" }, params._Texture);

    commandList.draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
}


OffscreenPassDraw::OffscreenPassDraw(bool isSwapchain) noexcept
    : m_OffscreenTarget(nullptr)
    , m_isSwapchain(isSwapchain)
{
}

OffscreenPassDraw::~OffscreenPassDraw()
{
    if (m_OffscreenTarget)
    {
        delete m_OffscreenTarget;
        m_OffscreenTarget = nullptr;
    }
}

const renderer::RenderTargetState* OffscreenPassDraw::GetRenderTarget() const
{
    return m_OffscreenTarget;
}

void OffscreenPassDraw::Init(renderer::CommandList& cmdList, const math::Dimension2D& size)
{
    if (m_isSwapchain)
    {
        m_OffscreenTarget = cmdList.createObject<renderer::RenderTargetState>(size, 0, "ShapchainScreen");
        m_OffscreenTarget->setColorTexture(0, cmdList.getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
            },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
        });
    }
}

void OffscreenPassDraw::Draw(renderer::CommandList& cmdList, DrawLists& drawList)
{
    drawList._Profiler->start("SceneLoop.OffscreenPassDraw");

    cmdList.setViewport(math::Rect32(0, 0, m_OffscreenTarget->getDimension().m_width, m_OffscreenTarget->getDimension().m_height));
    cmdList.setScissor(math::Rect32(0, 0, m_OffscreenTarget->getDimension().m_width, m_OffscreenTarget->getDimension().m_height));
    cmdList.setRenderTarget(m_OffscreenTarget);

    this->QueryTimeStamp(drawList._TimeStampQuery, 4, "OffscreenPassDraw start");

    drawList._Render->Render(cmdList, *std::get<0>(drawList._DrawState[0]), nullptr);

    this->QueryTimeStamp(drawList._TimeStampQuery, 5, "OffscreenPassDraw end");

    drawList._Profiler->stop("SceneLoop.OffscreenPassDraw");
}
