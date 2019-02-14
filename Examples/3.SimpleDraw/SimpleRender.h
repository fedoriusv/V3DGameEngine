#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/RenderTarget.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/Texture.h"

#include "Scene/ModelHelper.h"
#include "Scene/CameraHelper.h"


namespace v3d
{
namespace renderer
{

class SimpleRender
{
public:
    SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const scene::Model*> models) noexcept;
    ~SimpleRender();

    void update(u32 shaderIndex, renderer::CommandList& cmdList, const core::Vector3D& pos, const core::Vector3D& rotate);
    void render(renderer::CommandList& cmdList);

    void setCamera(scene::Camera* camera);

private:

    ShaderProgram* m_program[2];
    GraphicsPipelineState* m_pipeline[2];
    RenderTarget* m_renderTarget;

   scene::ModelHelper* m_modelDrawer;
   VertexStreamBuffer* m_drawBuffer;

   scene::Camera* m_camera;
};

} //namespace renderer
} //namespace v3d
