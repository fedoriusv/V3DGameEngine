#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/SamplerState.h"
#include "Scene/CameraHandler.h"
#include "FrameProfiler.h"

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
            _desc.clear();
            _desc.emplace_back(v3d::renderer::Descriptor({ &_constantBuffer, 0, sizeof(_constantBuffer) }, 0));

            cmdList.bindDescriptorSet(0, _desc);
        }

    private:

        std::vector<v3d::renderer::Descriptor> _desc;
    };

    virtual void setup(v3d::renderer::RenderTargetState* renderTarget, const v3d::scene::CameraHandler& camera) = 0;
    virtual void process(v3d::renderer::GraphicsPipelineState* pipeline,const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props) = 0;

    virtual void updateParameters(const std::function<void(Render*)>& callback) = 0;
    virtual void bindParameters(v3d::renderer::GraphicsPipelineState* pipeline) = 0;

    v3d::renderer::CmdListRender*     _CmdList;

    v3d::renderer::RenderTargetState* _RenderTarget;
    ViewportParameters                _ViewportParams;
};

class TextureRenderWorker : public Render
{
public:

    struct LightParameters
    {
        struct Light
        {
            v3d::math::Vector4D _lightPos;
            v3d::color::Color _color;
        };
        Light _constantBuffer;

        void bindUniformParameters(v3d::renderer::CmdListRender& cmdList, const v3d::renderer::ShaderProgram* program)
        {
            _desc.clear();
            _desc.emplace_back(v3d::renderer::Descriptor({ &_constantBuffer, 0, sizeof(_constantBuffer) }, 1));

            cmdList.bindDescriptorSet(0, _desc);
        }

    private:

        std::vector<v3d::renderer::Descriptor> _desc;
    };

    struct TextureUniformParameters
    {
        struct UniformBuffer
        {
            v3d::math::Matrix4D _modelMatrix;
            v3d::math::Matrix4D _normalMatrix;
        };
        UniformBuffer                _constantBufferVS;
        v3d::renderer::Texture2D*    _texture;
        v3d::renderer::SamplerState* _sampler;

        void bindUniformParameters(v3d::renderer::CmdListRender& cmdList, const v3d::renderer::ShaderProgram* program)
        {
            _desc.clear();
            _desc.emplace_back(v3d::renderer::Descriptor({ &_constantBufferVS, 0, sizeof(_constantBufferVS) }, 2));
            _desc.emplace_back(_sampler, 3);
            _desc.emplace_back(_texture, 4);

            cmdList.bindDescriptorSet(1, _desc);
        }

    private:

        std::vector<v3d::renderer::Descriptor> _desc;
    };

    void setup(v3d::renderer::RenderTargetState* renderTarget, const v3d::scene::CameraHandler& camera) override;
    void process(v3d::renderer::GraphicsPipelineState * pipeline, const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props) override;
    void updateParameters(const std::function<void(Render*)>& callback) override;
    void bindParameters(v3d::renderer::GraphicsPipelineState* pipeline) override;

public:

    LightParameters          _LightParams;
    TextureUniformParameters _TextureParams;
};
} //namespace app
