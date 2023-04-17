#include "BasePass.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"


BasePassDraw::TexturedRender::TexturedRender(const renderer::VertexInputAttributeDescription& desc) noexcept
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
    std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShaders<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "texture.hlsl",
        {
            {"main_VS", renderer::ShaderType::Vertex },
            {"main_FS", renderer::ShaderType::Fragment }
        });

    m_Program = commandList.createObject<renderer::ShaderProgram>(shaders);
    m_Pipeline = commandList.createObject<renderer::GraphicsPipelineState>(m_Desc, m_Program, renderTaget, "TexturedRenderPipeline");
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
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

BasePassDraw::OcclusionQuery::OcclusionQuery() noexcept
    : m_QueryPipeline(nullptr)
    , m_QueryProgram(nullptr)
    , m_OcclusionQuery(nullptr)
    , m_Desc(
        {
            renderer::VertexInputAttributeDescription::InputBinding(0,  renderer::VertexInputAttributeDescription::InputRate::InputRate_Vertex, sizeof(math::Vector3D))
        },
        {
            renderer::VertexInputAttributeDescription::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0)
        })
{
}

BasePassDraw::OcclusionQuery::~OcclusionQuery()
{
    ClearGeometry();
    delete m_QueryPipeline;
    delete m_QueryProgram;
}

void BasePassDraw::OcclusionQuery::ClearGeometry()
{
    delete m_OcclusionQuery;
    m_QueryResponse.clear();

    for (u32 i = 0; i < m_AABBList._DrawState.size(); ++i)
    {
        delete std::get<1>(m_AABBList._DrawState[i]);
    }
    m_AABBList._DrawState.clear();

    for (auto& buffer : m_vertexBuffer)
    {
        delete buffer;
    }
    m_vertexBuffer.clear();
}

void BasePassDraw::OcclusionQuery::Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget)
{
    const renderer::Shader* shader = resource::ResourceLoaderManager::getLazyInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(commandList.getContext(), "query.hlsl",
        renderer::ShaderType::Vertex, "main_VS");

    m_QueryProgram = commandList.createObject<renderer::ShaderProgram>(shader);
    m_QueryPipeline = commandList.createObject<renderer::GraphicsPipelineState>(m_Desc, m_QueryProgram, renderTaget, "QueryPipeline");
    m_QueryPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_QueryPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_QueryPipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_QueryPipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
    m_QueryPipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_QueryPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
    m_QueryPipeline->setDepthWrite(true);
    m_QueryPipeline->setDepthTest(true);
}

void BasePassDraw::OcclusionQuery::PrepareGeometry(renderer::CommandList& cmdList, DrawLists& drawList)
{
    if (m_QueryResponse.size() < drawList._DrawState.size())
    {
        ClearGeometry();

        m_QueryResponse.resize(drawList._DrawState.size(), true);
        for (u32 i = 0; i < drawList._DrawState.size(); ++i)
        {
            math::Vector3D positions[8];
            math::calculateVerticesFromAABB(std::get<1>(drawList._DrawState[i])->_AABB, positions);

            std::array<math::Vector3D, 36> vertices =
            {
                positions[0], positions[1], positions[2], positions[0], positions[2], positions[2],//front
                positions[4], positions[5], positions[6], positions[4], positions[6], positions[7],//back
                positions[0], positions[3], positions[4], positions[4], positions[3], positions[7],//left
                positions[1], positions[5], positions[2], positions[5], positions[6], positions[2],//right
                positions[3], positions[2], positions[6], positions[3], positions[6], positions[7],//top
                positions[0], positions[1], positions[5], positions[0], positions[5], positions[4],//bottom
            };
            renderer::VertexStreamBuffer* vertexBuffer = cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write, sizeof(math::Vector3D) * vertices.size(), vertices.data(), "QueryAABB");
            m_vertexBuffer.push_back(vertexBuffer);

            BaseDraw::MeshInfo* meshdata = new BaseDraw::MeshInfo
            {
                renderer::StreamBufferDescription(vertexBuffer, 0, 0),
                m_Desc,
                { 0, vertices.size(), 0, 1, false},
                std::get<1>(drawList._DrawState[i])->_AABB
            };
            m_AABBList._DrawState.push_back(std::make_tuple(std::get<0>(drawList._DrawState[i]), meshdata));
        }

        m_OcclusionQuery = cmdList.createObject<renderer::QueryOcclusionRequest>([this](const std::vector<u64>& samples, const std::vector<std::string>& tags)-> void
            {
                static const u32 k_samplesThreshold = 0;
                for (u32 index = 0; index < m_QueryResponse.size(); ++index)
                {
                    m_QueryResponse[index] = (samples[index] > k_samplesThreshold) ? true : false;
                }
            }, static_cast<u32>(m_QueryResponse.size()), true);
    }
    else
    {
        for (u32 i = 0; i < drawList._DrawState.size(); ++i)
        {
            std::get<0>(m_AABBList._DrawState[i]) = std::get<0>(drawList._DrawState[i]);
        }
    }
}

void BasePassDraw::OcclusionQuery::Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData)
{
    struct AABB
    {
        math::Matrix4D projectionMatrix;
        math::Matrix4D viewMatrix;
        math::Matrix4D modelMatrix;
    } constantBuffer;

    constantBuffer.projectionMatrix = params._ConstantBuffer.projectionMatrix;
    constantBuffer.viewMatrix = params._ConstantBuffer.viewMatrix;
    constantBuffer.modelMatrix = params._ConstantBuffer.modelMatrix;

    m_QueryProgram->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "vs_buffer" }, 0, (u32)sizeof(AABB), & constantBuffer);

    commandList.draw(meshData->_BufferDescription, meshData->_DrawProperties._start, meshData->_DrawProperties._count, meshData->_DrawProperties._countInstance);
}

void BasePassDraw::OcclusionQuery::DrawOcclusionTest(renderer::CommandList& commandList, DrawLists& drawList)
{
    drawList._Profiler->start("SceneLoop.BasePassDraw.DrawOcclusionTest");
    commandList.setPipelineState(m_QueryPipeline);

    drawList._TimeStampQuery->timestampQuery(6);
    for (u32 i = 0; i < m_AABBList._DrawState.size(); ++i)
    {
        m_OcclusionQuery->beginQuery(i);

        const ProgramParams& params = *std::get<0>(drawList._DrawState[i]);
        const MeshInfo* meshData = std::get<1>(m_AABBList._DrawState[i]);
        Render(commandList, params, meshData);

        m_OcclusionQuery->endQuery(i);
    }
    drawList._TimeStampQuery->timestampQuery(7);

    drawList._Profiler->stop("SceneLoop.BasePassDraw.DrawOcclusionTest");
}

void BasePassDraw::OcclusionQuery::UpdateVisibleList(const DrawLists& dawLists, DrawLists& visibleDrawList)
{
    dawLists._Profiler->start("BasePassDraw.UpdateVisibleList");

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

void BasePassDraw::Init(renderer::CommandList& cmdList, const math::Dimension2D& size)
{
    m_ColorAttachment = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_R8G8B8A8_UNorm,
        size, renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
    m_DepthAttachment = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt,
        size, renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");

    m_RenderTarget = cmdList.createObject<renderer::RenderTargetState>(size, 0, "RenderTarget");
    m_RenderTarget->setColorTexture(0, m_ColorAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        });
    m_RenderTarget->setDepthStencilTexture(m_DepthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    if (_EnableQuery)
    {
        m_QueryTest = new OcclusionQuery();
        m_QueryTest->Init(cmdList, m_RenderTarget);
    }
}

void BasePassDraw::Draw(renderer::CommandList& cmdList, DrawLists& drawList)
{
    drawList._Profiler->start("SceneLoop.BasePassDraw");
    //std::this_thread::sleep_for(std::chrono::duration<double,std::milli>(200));

    cmdList.setViewport(math::Rect32(0, 0, m_RenderTarget->getDimension().m_width, m_RenderTarget->getDimension().m_height));
    cmdList.setScissor(math::Rect32(0, 0, m_RenderTarget->getDimension().m_width, m_RenderTarget->getDimension().m_height));
    cmdList.setRenderTarget(m_RenderTarget);

    DrawLists visibleDrawList = drawList;
    if (_EnableQuery)
    {
        m_QueryTest->PrepareGeometry(cmdList, drawList);
        m_QueryTest->DrawOcclusionTest(cmdList, drawList);
        m_QueryTest->UpdateVisibleList(drawList, visibleDrawList);
    }

    m_RenderTarget->clearAttachments(renderer::TargetRegion(m_RenderTarget->getDimension().m_width, m_RenderTarget->getDimension().m_height), math::Vector4D(0.0f), 0.f);

    this->QueryTimeStamp(drawList._TimeStampQuery, 2, "BasePassDraw start");

    RenderPolicy* render = drawList._Render;
    for (u32 i = 0; i < visibleDrawList._DrawState.size(); ++i)
    {
        const ProgramParams& params = *std::get<0>(visibleDrawList._DrawState[i]);
        const MeshInfo* meshData = std::get<1>(visibleDrawList._DrawState[i]);
        render->Render(cmdList, params, meshData);
    }
    drawList._DrawedLastFrame = visibleDrawList._DrawState.size();

    this->QueryTimeStamp(drawList._TimeStampQuery, 3, "BasePassDraw end");
    drawList._Profiler->stop("SceneLoop.BasePassDraw");
}


