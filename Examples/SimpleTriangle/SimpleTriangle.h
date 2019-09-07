#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"

#include "Scene/Camera.h"

class SimpleTriangle
{
public:

    SimpleTriangle();
    ~SimpleTriangle();

    void init(v3d::renderer::CommandList* m_CommandList, const v3d::core::Dimension2D& size);
    void update();
    void render();
    void terminate();

private:

    v3d::renderer::CommandList* m_CommandList;

    v3d::renderer::ShaderProgram* m_Program;
    v3d::renderer::RenderTargetState* m_RenderTarget;
    v3d::renderer::GraphicsPipelineState* m_Pipeline;

    v3d::scene::Camera* m_Camera;
};