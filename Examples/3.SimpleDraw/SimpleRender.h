#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/RenderTarget.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/Texture.h"

namespace v3d
{

class SimpleRender
{
public:
    SimpleRender(renderer::CommandList& cmdList, const std::vector<f32>& geomentry);
    ~SimpleRender();

    void render(renderer::CommandList& cmdList);

private:

    renderer::Texture2D*        m_texture;
    renderer::RenderTarget*    m_renderTarget;
    renderer::ShaderProgram*   m_program;
    renderer::GraphicsPipelineState* m_pipeline;
    renderer::VertexStreamBuffer* m_vetexBuffer;
};

} //namespace v3d
