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

    struct Parameter
    {
        std::string _name;
        u32         _size;
        void*       _data;
    };

    SimpleRender(renderer::CommandList& cmdList, const core::Dimension2D& size, const std::vector<const Shader*> shaders, const std::vector<const resource::Image*> image, 
        const std::vector<const scene::Model*> models) noexcept;
    ~SimpleRender();

    void render(renderer::CommandList& cmdList);

    void setCamera(scene::Camera* camera);

    void updateParameter(renderer::CommandList& cmdList, const std::string& name, u32 size, const void* ubo);
    void updateParameter(renderer::CommandList& cmdList, const std::string& name, const resource::Image* image);

private:

    ShaderProgram*          m_program;
    GraphicsPipelineState*  m_pipeline;
    RenderTargetState*      m_renderTarget;

   scene::ModelHelper*      m_modelDrawer;

   scene::Camera*           m_camera;

   std::map<const resource::Image*, std::pair<Texture2D*, SamplerState*>> m_images;

   Texture2D*    m_texture;
   utils::IntrusivePointer<SamplerState> m_sampler;
};

} //namespace renderer
} //namespace v3d
