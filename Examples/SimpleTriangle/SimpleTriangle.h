#pragma once

#include "Common.h"
#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/Buffer.h"
#include "Renderer/Swapchain.h"

#include "Scene/CameraArcballHandler.h"
#include "Scene/Camera.h"

class SimpleTriangle
{
public:

    SimpleTriangle();
    ~SimpleTriangle();

    void init(v3d::renderer::Device* m_Device, v3d::renderer::Swapchain* m_Swapchain);
    void update(v3d::f32 dt);
    void render();
    void terminate();

    static bool dispatchEvent(SimpleTriangle* render, v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    v3d::renderer::Device* m_Device;
    v3d::renderer::CmdListRender* m_CmdList;
    v3d::math::Rect32 m_Rect;

    v3d::renderer::ShaderProgram* m_Program;
    v3d::renderer::RenderTargetState* m_RenderTarget;
    v3d::renderer::GraphicsPipelineState* m_Pipeline;

    v3d::renderer::VertexBuffer* m_Geometry;

    v3d::scene::CameraArcballHandler* m_Camera;
};