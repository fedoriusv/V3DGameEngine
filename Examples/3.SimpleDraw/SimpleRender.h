#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/RenderTarget.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/Texture.h"
#include "Scene/CameraHelper.h"

namespace v3d
{
namespace renderer
{

class SimpleRender
{
public:
    SimpleRender(renderer::CommandList& cmdList, const renderer::VertexInputAttribDescription& desc, const std::vector<f32>& geomentry);
    ~SimpleRender();

    void update(const core::Vector3D& pos, const core::Vector3D& rotate);
    void render(renderer::CommandList& cmdList);

private:

    scene::CameraHelper* m_camera;

    RenderTarget*     m_renderTarget;
    ShaderProgram*    m_program;
    GraphicsPipelineState* m_pipeline;
    VertexStreamBuffer* m_vetexBuffer;
};

} //namespace renderer
} //namespace v3d
