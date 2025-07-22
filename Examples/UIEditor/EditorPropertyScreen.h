#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/Widgets.h"

#include "EditorScene.h"

using namespace v3d;

class EditorPropertyScreen final
{
public:

    EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorPropertyScreen();

    void registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData);

    void build();
    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    ui::WidgetWindow* m_window;

    scene::SceneData* m_sceneData;
    scene::DrawInstanceDataState* m_selectedObject;

    std::array<ui::WidgetInputDragFloat*, 3> m_propertyPosition;
    std::array<ui::WidgetInputDragFloat*, 3> m_propertyRotation;
    std::array<ui::WidgetInputDragFloat*, 3> m_propertyScale;

    bool m_loaded;
};
