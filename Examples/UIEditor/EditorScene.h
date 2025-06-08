#pragma once

#include "Common.h"
#include "Utils/Observable.h"

#include "Scene/Scene.h"
#include "Renderer/Device.h"
#include "Renderer/RenderTargetState.h"
#include "RenderTechniques/RenderPipelineStage.h"

#include "Scene/Camera/CameraEditorHandler.h"
#include "Scene/Camera/Camera.h"

#include "UI/WidgetGroups.h"

using namespace v3d;

struct EditorReport
{
    scene::Transform transform;
};

class EditorScene : public scene::Scene, public event::InputEventHandler, public utils::Reporter<EditorReport>
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

    void modifyObject(const scene::Transform& transform);
    void selectObject(u32 i);

    void test_setOpacity(f32 op);
    void test_initContent(ui::WidgetListBox* list);

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

    struct SelectedObjects
    {
        scene::Transform _modelTransform;
        s32 _activeIndex = -1;
    };

    scene::CameraEditorHandler*     m_camera;
    math::Rect                      m_currentViewportRect;
    ViewportParams                  m_vewportParams;

    SelectedObjects                 m_selectedObjects;

    ui::WidgetListBox* m_contentList;
};
