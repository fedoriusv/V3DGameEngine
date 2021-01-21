#pragma once

#include "Common.h"

#include "Platform/Window.h"
#include "Event/InputEventHandler.h"

#include "Renderer/Context.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"

#include "Scene/ModelHelper.h"
#include "Scene/CameraArcballHelper.h"


class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

private:

    void Initialize();
    bool Running();
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    v3d::scene::CameraArcballHelper* m_Camera;

    //pass 0
    v3d::renderer::RenderTargetState* m_RenderTarget;
    v3d::renderer::Texture2D* m_ColorAttachment;
    v3d::renderer::Texture2D* m_DepthAttachment;

    v3d::renderer::GraphicsPipelineState* m_Pipeline;
    v3d::renderer::ShaderProgram* m_Program;

    v3d::renderer::SamplerState* m_Sampler;
    v3d::renderer::Texture2D* m_Texture;
    v3d::scene::ModelHelper* m_Geometry;

    //pass 1 (offscreen)
    v3d::renderer::RenderTargetState* m_OffscreenRenderTarget;

    v3d::renderer::GraphicsPipelineState* m_OffscreenPipeline;
    v3d::renderer::ShaderProgram* m_OffscreenProgram;
};
