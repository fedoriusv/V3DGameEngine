#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"

#include "Renderer/CommandList.h"

#include "Renderer/BufferProperties.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/SamplerState.h"

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
        core::Matrix4D _viewMatrix;
        core::Matrix4D _modelMatrix;
        core::Matrix4D _normalMatrix;
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

    TextureRender(renderer::CommandList& cmdList, const v3d::core::Dimension2D& viewport, const std::vector<const renderer::Shader*>& shaders, const renderer::VertexInputAttribDescription& vertex) noexcept;
    ~TextureRender();

    void process(renderer::CommandList& cmdList, const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& props);
    void updateParameters(renderer::CommandList& cmdList, const std::function<void(TextureUniformParameters&)>& callback);

private:

    TextureUniformParameters m_shaderParameters;

    utils::IntrusivePointer<renderer::ShaderProgram>          m_program;
    utils::IntrusivePointer<renderer::GraphicsPipelineState>  m_pipeline;
    utils::IntrusivePointer<renderer::RenderTargetState>      m_renderTarget;

    utils::IntrusivePointer<renderer::GraphicsPipelineState>  m_pipelineMSAA;
    utils::IntrusivePointer<renderer::RenderTargetState>      m_renderTargetMSAA;

    utils::IntrusivePointer<renderer::SamplerState>           m_Sampler;
    utils::IntrusivePointer<renderer::ShaderProgram>          m_programBackbuffer;
    utils::IntrusivePointer<renderer::GraphicsPipelineState>  m_pipelineBackbuffer;
    utils::IntrusivePointer<renderer::RenderTargetState>      m_renderTargetBackbuffer;

    bool m_enableMSAA = true;
};

} //namespace v3d
