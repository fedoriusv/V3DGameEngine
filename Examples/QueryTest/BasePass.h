#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/Texture.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/QueryRequest.h"

#include "BaseDraw.h"

using namespace v3d;

class BasePassDraw : public BaseDraw
{
public:

    class TexturedRender : public RenderPolicy
    {
    public:

        explicit TexturedRender(const renderer::VertexInputAttribDescription& desc) noexcept;
        ~TexturedRender();

        void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) override;
        void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) override;

    private:

        renderer::GraphicsPipelineState* m_Pipeline;
        renderer::ShaderProgram* m_Program;
        renderer::VertexInputAttribDescription m_Desc;
    };

    class OcclusionQuery : public RenderPolicy
    {
    public:

        explicit OcclusionQuery(const renderer::VertexInputAttribDescription& desc, u32 tests) noexcept;
        ~OcclusionQuery();

        void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) override;
        void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) override;

        void DrawOcclusionTest(renderer::CommandList& cmdList, DrawLists& drawList);
        void UpdateVisibleList(const DrawLists& dawLists, DrawLists& visibleDrawList);

    private:

        renderer::GraphicsPipelineState* m_QueryPipeline;
        renderer::ShaderProgram* m_QueryProgram;
        renderer::VertexInputAttribDescription m_DescTemp;

        renderer::QueryOcclusionRequest* m_OcclusionQuery;
        std::vector<bool> m_QueryResponse;

        DrawLists m_AABBList;
    };

    BasePassDraw() noexcept;
    ~BasePassDraw();

    const renderer::RenderTargetState* GetRenderTarget() const override;

    void Init(renderer::CommandList& cmdList, const core::Dimension2D& size)override;
    void Draw(renderer::CommandList& cmdList, DrawLists& drawList) override;

private:

    renderer::RenderTargetState* m_RenderTarget;
    renderer::Texture2D* m_ColorAttachment;
    renderer::Texture2D* m_DepthAttachment;

    OcclusionQuery* m_QueryTest;
};