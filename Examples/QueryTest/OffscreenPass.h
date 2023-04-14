#pragma once

#include "Common.h"
#include "Renderer/RenderTargetState.h"

#include "BaseDraw.h"

using namespace v3d;

class OffscreenPassDraw : public BaseDraw
{
public:

    class OffsceenRender : public RenderPolicy
    {
    public:

        OffsceenRender() noexcept;
        ~OffsceenRender();

        void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) override;
        void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) override;

    private:

        renderer::GraphicsPipelineState* m_OffscreenPipeline;
        renderer::ShaderProgram* m_OffscreenProgram;
    };

    explicit OffscreenPassDraw(bool isSwapchain) noexcept;
    ~OffscreenPassDraw();

    const renderer::RenderTargetState* GetRenderTarget() const override;

    void Init(renderer::CommandList& cmdList, const math::Dimension2D& size)override;
    void Draw(renderer::CommandList& cmdList, DrawLists& drawList) override;

private:

    renderer::RenderTargetState* m_OffscreenTarget;
    bool m_isSwapchain;
};