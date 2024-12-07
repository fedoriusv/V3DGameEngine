#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/SamplerState.h"
#include "Scene/CameraHandler.h"

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

class Render
{
public:

    struct ViewportParameters
    {
        struct UniformBuffer
        {
            v3d::math::Matrix4D _projectionMatrix;
            v3d::math::Matrix4D _viewMatrix;
            v3d::math::Vector4D _cameraPosition;
            v3d::math::Vector4D _viewportSize;
        };
        UniformBuffer _constantBuffer;

        void bindUniformParameters(v3d::renderer::CmdListRender& cmdList, const v3d::renderer::ShaderProgram* program)
        {
            v3d::renderer::Descriptor viewport({ &_constantBuffer, 0, sizeof(_constantBuffer) }, 0);
            cmdList.bindDescriptorSet(0, { viewport });
        }
    };

    virtual void setup(v3d::renderer::RenderTargetState* renderTarget, const v3d::scene::CameraHandler& camera) = 0;
    virtual void process(v3d::renderer::GraphicsPipelineState* pipeline,const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props) = 0;

    v3d::renderer::CmdListRender*     _CmdList;

    v3d::renderer::RenderTargetState* _RenderTarget;
    ViewportParameters                _ViewportParams;
};

class TextureRenderWorker : public Render
{
public:

    struct TextureUniformParameters
    {
        TextureUniformParameters() noexcept;
        ~TextureUniformParameters() = default;

        struct UniformBuffer
        {
            v3d::math::Matrix4D _modelMatrix;
            v3d::math::Matrix4D _normalMatrix;
        };

        struct Light
        {
            v3d::math::Vector4D _lightPos;
        };

        UniformBuffer           _constantBufferVS;
        Light                   _constantBufferFS;
        v3d::renderer::Texture2D* _texture;
        v3d::renderer::SamplerState* _sampler;

        void bindUniformParameters(v3d::renderer::CmdListRender& cmdList, const v3d::renderer::ShaderProgram* program)
        {
            v3d::renderer::Descriptor vsCBO({ &_constantBufferVS, 0, sizeof(_constantBufferVS) }, 0);

            v3d::renderer::Descriptor sampler(_sampler, 1);
            v3d::renderer::Descriptor texture(_texture, 2);
            v3d::renderer::Descriptor fsCBO({ &_constantBufferFS, 0, sizeof(_constantBufferFS) }, 3);

            cmdList.bindDescriptorSet(0, { vsCBO, fsCBO, texture, sampler });
        }
    };

    void setup(v3d::renderer::RenderTargetState* renderTarget, const v3d::scene::CameraHandler& camera) override;
    void process(v3d::renderer::GraphicsPipelineState * pipeline, const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props) override;

public:

    TextureUniformParameters _TextureParams;
};
} //namespace app
