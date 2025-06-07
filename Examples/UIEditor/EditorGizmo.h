#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/WidgetGizmo.h"

using namespace v3d;

class EditorGizmo : public event::InputEventHandler
{
public:

    EditorGizmo();
    ~EditorGizmo();

    void init(ui::WidgetGizmo* widget);
    void modify(const scene::Transform& transform);
    void select();

    void setEnable(bool enable);
    void setOperation(u32 index);

    void update(f32 dt);

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    ui::WidgetGizmo* m_gizmo;
};
