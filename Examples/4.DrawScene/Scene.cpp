#include "Scene.h"

#include "Renderer/Formats.h"
#include "Renderer/Shader.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/StreamBuffer.h"

#include "Resource/Image.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

#include "Scene/Model.h"
#include "Scene/Camera.h"

namespace v3d
{
namespace scene
{

Scene::Scene() noexcept
{

}
    
Scene::~Scene()
{

}

void Scene::onUpdate()
{
}

void Scene::onRender(v3d::renderer::CommandList & cmd)
{
}

void Scene::onLoad(v3d::renderer::CommandList & cmd)
{
    //Load Sponza
    Model* sponza = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/4.DrawScene/data/sponza.dae");

    //Load skysphere
    Model* skysphereModel = resource::ResourceLoaderManager::getInstance()->load<Model, resource::ModelFileLoader>("examples/4.drawscene/data/skysphere.dae");
    renderer::Shader* skysphereVertShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/shaders/skysphere.vert");
    renderer::Shader* skysphereFragShader = resource::ResourceLoaderManager::getInstance()->loadShader<renderer::Shader, resource::ShaderSourceFileLoader>(cmd.getContext(), "examples/4.drawscene/shaders/skysphere.frag");

}


} //namespace scene
} //namespace v3d
