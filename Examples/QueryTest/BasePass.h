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

        explicit TexturedRender(const renderer::VertexInputAttributeDescription& desc) noexcept;
        ~TexturedRender();

        void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) override;
        void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) override;

    private:

        renderer::GraphicsPipelineState* m_Pipeline;
        renderer::ShaderProgram* m_Program;
        renderer::VertexInputAttributeDescription m_Desc;
    };

    class OcclusionQuery : public RenderPolicy
    {
    public:

        explicit OcclusionQuery() noexcept;
        ~OcclusionQuery();

        void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) override;
        void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) override;

        void PrepareGeometry(renderer::CommandList& cmdList, DrawLists& drawList);
        void DrawOcclusionTest(renderer::CommandList& cmdList, DrawLists& drawList);
        void UpdateVisibleList(const DrawLists& dawLists, DrawLists& visibleDrawList);

    private:

        void ClearGeometry();

        renderer::GraphicsPipelineState* m_QueryPipeline;
        renderer::ShaderProgram* m_QueryProgram;
        renderer::VertexInputAttributeDescription m_Desc;

        renderer::QueryOcclusionRequest* m_OcclusionQuery;
        std::vector<bool> m_QueryResponse;

        DrawLists m_AABBList;
        std::vector<renderer::VertexStreamBuffer*> m_vertexBuffer;
    };

    BasePassDraw() noexcept;
    ~BasePassDraw();

    const renderer::RenderTargetState* GetRenderTarget() const override;

    void Init(renderer::CommandList& cmdList, const math::Dimension2D& size)override;
    void Draw(renderer::CommandList& cmdList, DrawLists& drawList) override;

private:

    renderer::RenderTargetState* m_RenderTarget;
    renderer::Texture2D* m_ColorAttachment;
    renderer::Texture2D* m_DepthAttachment;

    OcclusionQuery* m_QueryTest;
};