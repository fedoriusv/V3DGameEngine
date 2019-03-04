#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class GraphicsPipelineState;
    class ShaderProgram;

} //namespace renderer
namespace scene
{
    class Camera;
    class ModelHelper;
    struct MaterialHeader;

    class Scene
    {
    public:
        Scene(const core::Dimension2D& size) noexcept;
        ~Scene();

        void onLoad(v3d::renderer::CommandList & cmd);

        void onUpdate();
        void onRender(v3d::renderer::CommandList& cmd);

        void setCamera(scene::Camera* camera);

    private:

        renderer::RenderTargetState* m_simpleBackbuffer;
        renderer::ShaderProgram* m_simpleProgram;
        renderer::GraphicsPipelineState* m_simplePipeline;

        //void updateProgramParameters(renderer::ShaderProgram* program, scene::ModelHelper* model, scene::MaterialHelper* material);

        renderer::RenderTargetState* m_offsceenRenderTarget;



        renderer::GraphicsPipelineState* m_sponzaMRTPipeline;
        renderer::ShaderProgram* m_sponzaMRTProgram;

        scene::MaterialHeader* m_sponzaMaterial;
        renderer::GraphicsPipelineState* m_mptParticlesPipeline;
        renderer::GraphicsPipelineState* m_mptSkyboxPipeline;

        scene::ModelHelper* m_modelDrawer;

        core::Dimension2D m_size;
        scene::Camera*    m_camera;
    };

} //namespace scene
} //namespace v3d
