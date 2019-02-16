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

    struct Parameter
    {
        std::string _name;
        u32         _size;
        void*       _data;
    };

    SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const scene::Model*> models) noexcept;
    ~SimpleRender();

    void update(renderer::CommandList& cmdList);
    void render(renderer::CommandList& cmdList);

    void setCamera(scene::Camera* camera);

private:

    void updateParameters(renderer::CommandList& cmdList, const std::vector<Parameter>& parameters);

    ShaderProgram*          m_program;
    GraphicsPipelineState*  m_pipeline;

    RenderTarget*           m_renderTarget;

   scene::ModelHelper*      m_modelDrawer;

   scene::Camera*           m_camera;
};

} //namespace renderer
} //namespace v3d
