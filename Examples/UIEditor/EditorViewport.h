#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Events/Input/InputEventHandler.h"

#include "Renderer/Device.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/PipelineState.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/Buffer.h"
#include "Renderer/Swapchain.h"
#include "Renderer/Texture.h"

#include "Scene/CameraEditorHandler.h"
#include "Scene/Camera.h"

#include "Scene.h"

using namespace v3d;

class EditorViewport : public event::InputEventHandler
{
public:

    struct ViewportParams
    {
        f32 _fov  = 45.f;
        f32 _near = 0.01f;
        f32 _far  = 10000.f;
    };

    EditorViewport();
    ~EditorViewport();

    void init(renderer::Device* device, const math::Dimension2D& viewportSize);
    void cleanup();

    void update(f32 dt);
    void render(renderer::CmdListRender* cmdList);

    const renderer::Texture2D* getOutputTexture() const;
    const math::Rect32&        getViewportArea() const;
    scene::Camera*             getCamera();

public:

    void onChanged(const math::Rect32& viewport);

private:

    void recreateViewport(const math::Dimension2D& viewportSize);

    renderer::Device*                  m_Device;

    scene::CameraEditorHandler*        m_Camera;
    renderer::RenderTargetState*       m_VewiportTarget;
    math::Rect32                       m_CurrentViewportRect;
    ViewportParams                     m_VewportParams;

    Scene::DrawData                    m_drawState;
};