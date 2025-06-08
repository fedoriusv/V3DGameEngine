#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/WidgetGroups.h"

using namespace v3d;

class EditorContentScreen : public event::InputEventHandler
{
public:

    EditorContentScreen();
    ~EditorContentScreen();

    void init(ui::WidgetListBox* widget);
    void modify(const scene::Transform& transform);
    void select();

    void update(f32 dt);

private:

    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    ui::WidgetListBox* m_list;
};