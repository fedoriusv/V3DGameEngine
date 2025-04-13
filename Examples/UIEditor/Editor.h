#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"
#include "Events/InputEventMouse.h"
#include "Events/InputEventKeyboard.h"

#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/Buffer.h"
#include "Renderer/Swapchain.h"
#include "Renderer/Texture.h"

#include "Scene/CameraArcballHandler.h"
#include "Scene/Camera.h"

class EditorScene : public v3d::event::InputEventHandler
{
public:

    EditorScene();
    ~EditorScene();

    void init(v3d::renderer::Device* device, const v3d::math::Dimension2D& viewportSize);
    void cleanup();

    void update(v3d::f32 dt);
    void render(v3d::renderer::CmdListRender* cmdList);

    const v3d::renderer::Texture2D* getOutputTexture() const;
    const v3d::math::Rect32&        getViewportArea() const;


public:

    void onChanged(const v3d::platform::Window* window, const v3d::math::Rect32& viewport);

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    void recreateViewport(const v3d::math::Dimension2D& viewportSize);

    v3d::renderer::Device*                  m_Device;

    v3d::scene::CameraArcballHandler*       m_Camera;
    v3d::renderer::RenderTargetState*       m_VewiportTarget;

    const v3d::platform::Window*            m_CurrentWindow;
    v3d::math::Rect32                       m_CurrentViewportRect;

    v3d::renderer::ShaderProgram*           m_Program;
    v3d::renderer::GraphicsPipelineState*   m_Pipeline;
    v3d::renderer::VertexBuffer*            m_Geometry;

};