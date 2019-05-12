#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/SamplerState.h"

#include "Resource/Image.h"

#include "Scene/ModelHelper.h"
#include "Scene/CameraHelper.h"


namespace v3d
{
namespace renderer
{
class SimpleRender
{
public:

    SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const resource::Image*> image, 
        const std::vector<const scene::Model*> models) noexcept;
    ~SimpleRender();

    void render(renderer::CommandList& cmdList);

    void setCamera(scene::Camera* camera);

    void updateParameter(renderer::CommandList& cmdList, const std::string& name, u32 size, const void* ubo);
    void updateParameter(renderer::CommandList& cmdList, const std::string& name, u32 index);

private:

    utils::IntrusivePointer<ShaderProgram>          m_program;
    utils::IntrusivePointer<GraphicsPipelineState>  m_pipeline;
    utils::IntrusivePointer<RenderTargetState>      m_renderTarget;

   utils::IntrusivePointer<Texture2D>    m_texture;
   utils::IntrusivePointer<SamplerState> m_sampler;

   scene::ModelHelper*      m_modelDrawer;

   scene::Camera*           m_camera;
};

} //namespace renderer
} //namespace v3d
