#include "Render.h"

#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"
#include "Stream/StreamManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderCompiler.h"
#include "Resource/Loader/ShaderSourceStreamLoader.h"


using namespace v3d;

namespace app
{

void TextureRenderWorker::setup(v3d::renderer::RenderTargetState* renderTarget, const v3d::scene::CameraHandler& camera)
{
    _ViewportParams._constantBuffer._cameraPosition = v3d::math::Vector4D(camera.getPosition().getX(), camera.getPosition().getY(), camera.getPosition().getZ(), 0.0f);
    _ViewportParams._constantBuffer._projectionMatrix = camera.getCamera().getProjectionMatrix();
    _ViewportParams._constantBuffer._viewMatrix = camera.getCamera().getViewMatrix();
    _ViewportParams._constantBuffer._viewportSize = { (v3d::f32)renderTarget->getRenderArea()._width, (v3d::f32)renderTarget->getRenderArea()._height, 0.f, 0.f };

    _RenderTarget = renderTarget;
}

void TextureRenderWorker::updateParameters(const std::function<void(Render*)>& callback)
{
    callback(this);
}

void TextureRenderWorker::bindParameters(v3d::renderer::GraphicsPipelineState* pipeline)
{
    _ViewportParams.bindUniformParameters(*_CmdList, pipeline->getShaderProgram());
    _LightParams.bindUniformParameters(*_CmdList, pipeline->getShaderProgram());
}

void TextureRenderWorker::process(v3d::renderer::GraphicsPipelineState* pipeline, const std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, DrawProperties>>& props)
{
    for (auto& buffer : props)
    {
        _TextureParams.bindUniformParameters(*_CmdList, pipeline->getShaderProgram());

        const  DrawProperties& props = std::get<1>(buffer);
        if (props._indexDraws)
        {
            _CmdList->drawIndexed(std::get<0>(buffer), props._start, props._count, 0, 0, props._countInstance);
        }
        else
        {
            _CmdList->draw(std::get<0>(buffer), props._start, props._count, 0, props._countInstance);
        }
    }
}

} //namespace v3d
