#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/Widgets.h"

#include "EditorScene.h"

using namespace v3d;

class EditorViewScreen final : public EditorScreen
{
public:

    EditorViewScreen(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorViewScreen();

    void registerWiget(ui::Widget* widget, scene::SceneData& sceneData);
    void show();
    void hide();

    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    void build();

    event::GameEventReceiver* m_gameEventRecevier;
    ui::WidgetPopup* m_popup;

    scene::SceneData* m_sceneData;

    bool m_loaded;
};