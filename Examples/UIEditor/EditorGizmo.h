#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Game/GameEventHandler.h"
#include "UI/WidgetGizmo.h"

#include "EditorScene.h"

using namespace v3d;

class EditorGizmo final
{
public:

    EditorGizmo(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorGizmo();

    void registerWiget(ui::WidgetGizmo* widget, scene::SceneData& sceneData);

    void modify(const scene::Transform& transform);
    void select();

    void setEnable(bool enable);
    void setOperation(u32 index);

    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event);

private:

    ui::WidgetGizmo* m_gizmo;

    scene::SceneData* m_sceneData;
    scene::DrawInstanceDataState* m_selectedObject;
    s32 m_currentOp;
};
