#pragma once

#include "Common.h"

#include "Events/Input/InputEventHandler.h"
#include "Events/Game/GameEventHandler.h"
#include "Events/Game/GameEventReceiver.h"

#include "Renderer/Device.h"
#include "Renderer/RenderTargetState.h"
#include "RenderTechniques/RenderPipelineStage.h"

#include "Scene/Scene.h"
#include "Scene/Camera/CameraEditorHandler.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Material.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/ModelHandler.h"

#include "UI/WidgetHandler.h"
#include "UI/WidgetGroups.h"

using namespace v3d;

struct EditorSelectionEvent : event::GameEvent
{
    EditorSelectionEvent(scene::SceneNode* node) noexcept
        : event::GameEvent(GameEvent::GameEventType::SelectObject)
        , _node(node)
    {
    }

    virtual ~EditorSelectionEvent() = default;

    scene::SceneNode* _node;
};

struct EditorTrasformEvent : event::GameEvent
{
    EditorTrasformEvent(scene::SceneNode* node, scene::TransformMode mode, const scene::Transform& transform) noexcept
        : event::GameEvent(GameEvent::GameEventType::TransformObject)
        , _node(node)
        , _mode(mode)
        , _transform(transform)
    {
    }

    virtual ~EditorTrasformEvent() = default;

    scene::SceneNode* _node;
    scene::TransformMode _mode;
    scene::Transform _transform;
};

constexpr u32 k_emptyIndex = -1;


class EditorScreen
{
public:

    virtual void registerWiget(ui::Widget* widget, scene::SceneData& sceneData) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;

    virtual void update(f32 dt) = 0;

    virtual bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    virtual bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);
};


class EditorScene : public scene::Scene
{
public:

    class RenderPipelineScene : public scene::RenderTechnique
    {
    public:

        RenderPipelineScene(scene::ModelHandler* modelHandler, ui::WidgetHandler* uiHandler);
        ~RenderPipelineScene();
    };

    EditorScene() noexcept;
    ~EditorScene();

    void create(renderer::Device* device, const math::Dimension2D& viewportSize) final;
    void destroy() final;

    void preRender(f32 dt) final;
    void postRender(f32 dt) final;

    void submitRender() final;

public:

    void modifyObject(const math::Matrix4D& transform);

public:

    renderer::Texture2D* getOutputTexture() const;
    const math::Rect& getViewportArea() const;
    scene::Camera* getCamera();

    event::InputEventHandler* getInputHandler();
    event::GameEventHandler* getGameHandler();

    event::GameEventReceiver* getGameEventReceiver();

public:

    void onChanged(const math::Rect& viewport);
    void onChanged(const math::Matrix4D& view);

private:

    void loadResources();

    renderer::Device*               m_device;

    scene::ModelHandler*            m_modelHandler;
    ui::WidgetHandler*              m_UIHandler;
    scene::CameraEditorHandler*     m_cameraHandler;
    event::InputEventHandler*       m_inputHandler;
    event::GameEventHandler*        m_gameHandler;

    event::GameEventReceiver*       m_gameEventRecevier;
    RenderPipelineScene             m_mainPipeline;

    math::Rect                      m_currentViewportRect;

private:

    u32                             m_activeIndex;

    void editor_loadDebug();

    void test_loadLights();
    void test_loadScene(const std::string& name);
    void test_loadTestScene();

};
