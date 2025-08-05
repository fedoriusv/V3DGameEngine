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
#include "Scene/ModelHandler.h"

#include "UI/WidgetHandler.h"
#include "UI/WidgetGroups.h"

using namespace v3d;

struct EditorSelectionEvent : event::GameEvent
{
    EditorSelectionEvent(u32 selectedIndex) noexcept
        : event::GameEvent(GameEvent::GameEventType::SelectObject)
        , _selectedIndex(selectedIndex)
    {
    }

    virtual ~EditorSelectionEvent() = default;

    u32 _selectedIndex;
};

constexpr u32 k_emptyIndex = -1;


class EditorScene final
{
public:

    class RenderPipelineScene : public renderer::RenderTechnique
    {
    public:

        RenderPipelineScene(scene::ModelHandler* modelHandler, ui::WidgetHandler* uiHandler);
        ~RenderPipelineScene();
    };

    EditorScene() noexcept;
    ~EditorScene();

    void create(renderer::Device* device, const math::Dimension2D& viewportSize);
    void destroy();
    void beginFrame();
    void endFrame();

    void preRender(f32 dt);
    void postRender();

    void submitRender();

public:

    void modifyObject(const math::Matrix4D& transform);
    void selectObject(u32 i);

public:

    const renderer::Texture2D* getOutputTexture() const;
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

public:

    struct ViewportParams
    {
        f32 _fov = 45.f;
        f32 _near = 0.01f;
        f32 _far = 10000.f;
    };

    math::Rect                      m_currentViewportRect;
    ViewportParams                  m_vewportParams;

    u32                             m_activeIndex;
    u64                             m_frameCounter;

    scene::SceneData m_sceneData;
    std::vector<scene::FrameData> m_frameState;
    u32 m_stateIndex;

private:

    void editor_loadDebug(renderer::CmdListRender* cmdList);

    void test_loadCubes(renderer::CmdListRender* cmdList, u32 countOpaque, u32 countTransparency);
    void test_loadLights(renderer::CmdListRender* cmdList, u32 pointCount, u32 spotCount);

};
