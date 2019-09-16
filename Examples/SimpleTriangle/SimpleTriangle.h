#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/StreamBuffer.h"

#include "Scene/CameraArcballHelper.h"

#include "Scene/Camera.h"

class SimpleTriangle
{
public:

    SimpleTriangle();
    ~SimpleTriangle();

    void init(v3d::renderer::CommandList* m_CommandList, const v3d::core::Dimension2D& size);
    void update(v3d::f32 dt);
    void render();
    void terminate();

    static bool dispachEvent(SimpleTriangle* render, v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    v3d::renderer::CommandList* m_CommandList;

    v3d::renderer::ShaderProgram* m_Program;
    v3d::renderer::RenderTargetState* m_RenderTarget;
    v3d::renderer::GraphicsPipelineState* m_Pipeline;

    v3d::renderer::VertexStreamBuffer* m_Geometry;

    v3d::scene::CameraArcballHelper* m_Camera;
};