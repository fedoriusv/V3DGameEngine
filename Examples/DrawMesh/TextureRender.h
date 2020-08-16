#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"

#include "Renderer/BufferProperties.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"

namespace v3d
{

class TextureRender;

struct TextureUniformParameters
{
    TextureUniformParameters() noexcept;
    ~TextureUniformParameters() = default;

    struct UniformBuffer
    {
        core::Matrix4D _projectionMatrix;
        core::Matrix4D _modelMatrix;
        core::Matrix4D _viewMatrix;
    };

    struct Light
    {
        core::Vector4D _lightPos;
    };

    UniformBuffer           _constantBufferVS;
    Light                   _constantBufferFS;
    renderer::Texture2D*    _texture;
    renderer::SamplerState* _sampler;

private:

    friend TextureRender;

    void bindUniformParameters(renderer::CommandList& cmdList, renderer::ShaderProgram* program);
};


class TextureRender
{
public:

    TextureRender(renderer::CommandList& cmdList, const std::vector<const renderer::Shader*>& shaders, const renderer::VertexInputAttribDescription& vertex) noexcept;
    ~TextureRender();

    void process(renderer::CommandList& cmdList, const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& props);
    void updateParameters(renderer::CommandList& cmdList, const std::function<void(TextureUniformParameters&)>& callback);

private:

    TextureUniformParameters m_shaderParameters;

    utils::IntrusivePointer<renderer::ShaderProgram>          m_program;
    utils::IntrusivePointer<renderer::GraphicsPipelineState>  m_pipeline;
    utils::IntrusivePointer<renderer::RenderTargetState>      m_renderTarget;
};

} //namespace v3d
