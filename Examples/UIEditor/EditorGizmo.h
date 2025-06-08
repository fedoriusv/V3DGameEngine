#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/WidgetGizmo.h"

#include "EditorScene.h"

using namespace v3d;

class EditorGizmo : public event::InputEventHandler, public utils::Observer<EditorReport>
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

    void handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data) override;

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    ui::WidgetGizmo* m_gizmo;
};
