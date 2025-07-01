#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Game/GameEventHandler.h"
#include "UI/WidgetGizmo.h"

#include "EditorScene.h"

using namespace v3d;

class EditorGizmo : public event::InputEventHandler, public utils::Observer<EditorReport>
{
public:

    EditorGizmo() noexcept;
    ~EditorGizmo();

    void init(ui::WidgetGizmo* widget);
    void modify(const scene::Transform& transform);
    void select();

    void setEnable(bool enable);
    void setOperation(u32 index);

    void update(f32 dt);

private:

    void handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data) override;

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event);

    ui::WidgetGizmo* m_gizmo;

    scene::DrawInstanceData* m_selectedObject = nullptr;
};
