#pragma once

#include "Common.h"

#include "Scene/Scene.h"
#include "Renderer/Device.h"
#include "Renderer/RenderTargetState.h"
#include "RenderTechniques/RenderPipelineStage.h"

#include "Scene/Camera/CameraEditorHandler.h"
#include "Scene/Camera/Camera.h"

namespace v3d
{
class EditorScene : public scene::Scene, public event::InputEventHandler
{
public:

    class RenderPipelineScene : public renderer::RenderTechnique
    {
    public:

        RenderPipelineScene();
        ~RenderPipelineScene();
    };

    EditorScene() noexcept;
    ~EditorScene() = default;

    void create(renderer::Device* device, const math::Dimension2D& viewportSize);
    void destroy();

    void preRender(f32 dt);
    void postRender();

    void submitRender();

public:

    const renderer::Texture2D* getOutputTexture() const;
    const math::Rect& getViewportArea() const;
    scene::Camera* getCamera();

public:

    void onChanged(const math::Rect& viewport);
    void onChanged(const math::Matrix4D& view);

public:

    void loadResources();

    renderer::Device*   m_device;
    RenderPipelineScene m_pipeline;

public:

    struct ViewportParams
    {
        f32 _fov = 45.f;
        f32 _near = 0.01f;
        f32 _far = 10000.f;
    };

    scene::CameraEditorHandler*     m_camera;
    math::Rect                      m_currentViewportRect;
    ViewportParams                  m_vewportParams;
};

} //namespace v3d