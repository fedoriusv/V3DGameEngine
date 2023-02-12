#include "BasePass.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"


BasePassDraw::TexturedRender::TexturedRender(const renderer::VertexInputAttribDescription& desc) noexcept
    : m_Pipeline(nullptr)
    , m_Program(nullptr)
    , m_Desc(desc)
{
}

BasePassDraw::TexturedRender::~TexturedRender()
{
    delete m_Pipeline;
    delete m_Program;
}

void BasePassDraw::TexturedRender::Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget)
{
    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "texture.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_FS", renderer::ShaderType::Fragment }
        }, {}, resource::ShaderSource_UseDXCompiler);

    m_Program = commandList.createObject<renderer::ShaderProgram>(shaders);
    m_Pipeline = commandList.createObject<renderer::GraphicsPipelineState>(m_Desc, m_Program, renderTaget, "TexturedRenderPipeline");
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Equal);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
};

void BasePassDraw::TexturedRender::Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData)
{
    //static u32 s_i = 0;
    //if (s_i < 500)
    //{
    //    m_Pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
    //}
    //else
    //{
    //    m_Pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
    //}
    //++s_i;
    //if (s_i >= 1000)
    //{
    //    s_i = 0;
    //};

    commandList.setPipelineState(m_Pipeline);

    static const renderer::ShaderParam vs_buffer{ "vs_buffer" };
    m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>(vs_buffer, 0, (u32)sizeof(ProgramParams::UBO), &params._ConstantBuffer);

    static const renderer::ShaderParam colorSampler{ "colorSampler" };
    m_Program->bindSampler<renderer::ShaderType::Fragment>(colorSampler, params._Sampler);

    static const renderer::ShaderParam colorTexture{ "colorTexture" };
    m_Program->bindTexture<renderer::ShaderType::Fragment, renderer::Texture2D>(colorTexture, params._Texture);

    commandList.drawIndexed(meshData->_BufferDescription, meshData->_DrawProperties._start, meshData->_DrawProperties._count, meshData->_DrawProperties._countInstance);
}

BasePassDraw::OcclusionQuery::OcclusionQuery(const renderer::VertexInputAttribDescription& desc, u32 tests) noexcept
    : m_QueryPipeline(nullptr)
    , m_QueryProgram(nullptr)
    , m_OcclusionQuery(nullptr)
    , m_DescTemp(desc)
{
    m_QueryResponse.resize(tests, true);
}

BasePassDraw::OcclusionQuery::~OcclusionQuery()
{
    delete m_OcclusionQuery;
    delete m_QueryPipeline;
    delete m_QueryProgram;
}

void BasePassDraw::OcclusionQuery::Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget)
{
    //const renderer::Shader* shader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), 
    //    renderer::ShaderType::Vertex, "query.hlsl", "main_VS", {}, resource::ShaderSource_UseDXCompiler);

    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "query.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_FS", renderer::ShaderType::Fragment }
        }, {}, resource::ShaderSource_UseDXCompiler);

    m_QueryProgram = commandList.createObject<renderer::ShaderProgram>(shaders);
    m_QueryPipeline = commandList.createObject<renderer::GraphicsPipelineState>(m_DescTemp, m_QueryProgram, renderTaget, "QueryPipeline");
    m_QueryPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_QueryPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_QueryPipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_QueryPipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
    m_QueryPipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_QueryPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_QueryPipeline->setDepthWrite(true);
    m_QueryPipeline->setDepthTest(true);
    //TODO disable pixelshader

    m_OcclusionQuery = commandList.createObject<renderer::QueryOcclusionRequest>([this](const std::vector<u32>& samples, const std::vector<std::string>& tags)-> void
        {
            static const u32 k_samplesThreshold = 1;
            for (u32 index = 0; index < m_QueryResponse.size(); ++index)
            {
                m_QueryResponse[index] = (samples[index] > k_samplesThreshold) ? true : false;
            }
        }, static_cast<u32>(m_QueryResponse.size()));
}

void BasePassDraw::OcclusionQuery::Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData)
{
    commandList.setPipelineState(m_QueryPipeline);

    m_QueryProgram->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(ProgramParams::UBO), &params._ConstantBuffer);

    commandList.drawIndexed(meshData->_BufferDescription, meshData->_DrawProperties._start, meshData->_DrawProperties._count, meshData->_DrawProperties._countInstance);
}

void BasePassDraw::OcclusionQuery::DrawOcclusionTest(renderer::CommandList& commandList, DrawLists& drawList)
{
    drawList._Profiler->start("SceneLoop.BasePassDraw.DrawOcclusionTest");
    //drawList TODO get AABB
    m_AABBList._DrawState = drawList._DrawState;

    drawList._TimeStampQuery->timestampQuery(2);
    for (u32 i = 0; i < m_AABBList._DrawState.size(); ++i)
    {
        m_OcclusionQuery->beginQuery(i);

        const ProgramParams& params = *std::get<0>(drawList._DrawState[i]);
        const MeshInfo* meshData = std::get<1>(m_AABBList._DrawState[i]);
        Render(commandList, params, meshData);

        m_OcclusionQuery->endQuery(i);
    }
    drawList._TimeStampQuery->timestampQuery(3);

    drawList._Profiler->stop("SceneLoop.BasePassDraw.DrawOcclusionTest");
}

void BasePassDraw::OcclusionQuery::UpdateVisibleList(const DrawLists& dawLists, DrawLists& visibleDrawList)
{
    dawLists._Profiler->stop("BasePassDraw.UpdateVisibleList");

    visibleDrawList._DrawState.clear();
    visibleDrawList._DrawState.reserve(dawLists._DrawState.size());
    for (u32 i = 0; i < dawLists._DrawState.size(); ++i)
    {
        if (m_QueryResponse[i])
        {
            visibleDrawList._DrawState.push_back(dawLists._DrawState[i]);
        }
    }

    dawLists._Profiler->stop("BasePassDraw.UpdateVisibleList");
}

BasePassDraw::BasePassDraw() noexcept
    : m_RenderTarget(nullptr)
    , m_ColorAttachment(nullptr)
    , m_DepthAttachment(nullptr)
    , m_QueryTest(nullptr)
{
}

BasePassDraw::~BasePassDraw()
{
    if (m_QueryTest)
    {
        delete m_QueryTest;
    }

    delete m_RenderTarget;
    delete m_ColorAttachment;
    delete m_DepthAttachment;
}

const renderer::RenderTargetState* BasePassDraw::GetRenderTarget() const
{
    return m_RenderTarget;
}

void BasePassDraw::Init(renderer::CommandList& cmdList, const core::Dimension2D& size)
{
    m_ColorAttachment = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8G8B8A8_UNorm,
        size, renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
    m_DepthAttachment = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt,
        size, renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");

    m_RenderTarget = cmdList.createObject<renderer::RenderTargetState>(size, 0, "RenderTarget");
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

void BasePassDraw::Draw(renderer::CommandList& cmdList, DrawLists& drawList)
{
    drawList._Profiler->start("SceneLoop.BasePassDraw");
    //std::this_thread::sleep_for(std::chrono::duration<double,std::milli>(200));

    if (!m_QueryTest) //TODO
    {
        m_QueryTest = new OcclusionQuery(std::get<1>(drawList._DrawState[0])->_VertexLayoutDescription, drawList._DrawState.size());
        m_QueryTest->Init(cmdList, m_RenderTarget);

    }

    cmdList.setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    cmdList.setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    cmdList.setRenderTarget(m_RenderTarget);


    m_QueryTest->DrawOcclusionTest(cmdList, drawList);

    DrawLists visibleDrawList;
    m_QueryTest->UpdateVisibleList(drawList, visibleDrawList);

    drawList._TimeStampQuery->timestampQuery(4);
    RenderPolicy* render = drawList._Render;
    for (u32 i = 0; i < visibleDrawList._DrawState.size(); ++i)
    {
        const ProgramParams& params = *std::get<0>(visibleDrawList._DrawState[i]);
        const MeshInfo* meshData = std::get<1>(visibleDrawList._DrawState[i]);
        render->Render(cmdList, params, meshData);
    }
    drawList._DrawedLastFrame = visibleDrawList._DrawState.size();
    drawList._TimeStampQuery->timestampQuery(5);

    drawList._Profiler->stop("SceneLoop.BasePassDraw");
}


