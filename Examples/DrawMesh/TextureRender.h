#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/SamplerState.h"

namespace app
{

struct DrawProperties
{
    v3d::u32 _start;
    v3d::u32 _count;
    v3d::u32 _startInstance;
    v3d::u32 _countInstance;

    bool _indexDraws;
};

struct TextureUniformParameters
{
    TextureUniformParameters() noexcept;
    ~TextureUniformParameters() = default;

    struct UniformBuffer
    {
        v3d::math::Matrix4D _projectionMatrix;
        v3d::math::Matrix4D _viewMatrix;
        v3d::math::Matrix4D _modelMatrix;
        v3d::math::Matrix4D _normalMatrix;
    };

    struct Light
    {
        v3d::math::Vector4D _lightPos;
    };

    UniformBuffer           _constantBufferVS;
    Light                   _constantBufferFS;
    v3d::renderer::Texture2D*    _texture;
    v3d::renderer::SamplerState* _sampler;

    void bindUniformParameters(v3d::renderer::CmdListRender& cmdList, v3d::renderer::ShaderProgram* program);
};

class TextureRender
{
public:

    TextureRender(v3d::renderer::Device* device, v3d::renderer::CmdListRender& cmdList, v3d::renderer::Swapchain* swapchain, const std::vector<const v3d::renderer::Shader*>& shaders, const v3d::renderer::VertexInputAttributeDesc& vertex) noexcept;
    ~TextureRender();

    void process(v3d::renderer::CmdListRender& cmdList, const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props);
    void updateParameters(v3d::renderer::CmdListRender& cmdList, const std::function<void(TextureUniformParameters&)>& callback);

private:

    TextureUniformParameters m_shaderParameters;

    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram>          m_program;
    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState>  m_pipeline;
    v3d::utils::IntrusivePointer<v3d::renderer::RenderTargetState>      m_renderTarget;

    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState>  m_pipelineMSAA;
    v3d::utils::IntrusivePointer<v3d::renderer::RenderTargetState>      m_renderTargetMSAA;

    v3d::utils::IntrusivePointer<v3d::renderer::SamplerState>           m_Sampler;
    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram>          m_programBackbuffer;
    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState>  m_pipelineBackbuffer;
    v3d::utils::IntrusivePointer<v3d::renderer::RenderTargetState>      m_renderTargetBackbuffer;

    bool m_enableMSAA = true;
};

} //namespace v3d
