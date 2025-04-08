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

class EditorScene
{
public:

    EditorScene();
    ~EditorScene();

    void init(v3d::renderer::Device* m_Device, v3d::renderer::Swapchain* m_Swapchain, const v3d::renderer::RenderPassDesc& renderpassDesc);
    void update(v3d::f32 dt);
    void render(v3d::renderer::CmdListRender* cmdList, const v3d::math::Rect32& viewport);
    void terminate();

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    v3d::renderer::Device*                  m_Device;

    v3d::scene::CameraArcballHandler*       m_Camera;

    v3d::renderer::ShaderProgram*           m_Program;
    v3d::renderer::GraphicsPipelineState*   m_Pipeline;
    v3d::renderer::VertexBuffer*            m_Geometry;

};