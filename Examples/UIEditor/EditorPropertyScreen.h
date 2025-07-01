#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/Widgets.h"

#include "EditorScene.h"

using namespace v3d;

class EditorPropertyScreen : public event::InputEventHandler, public utils::Observer<EditorReport>
{
public:

    EditorPropertyScreen() noexcept;
    ~EditorPropertyScreen();

    void init(ui::WidgetWindow* widget);
    void build();
    void update(f32 dt);

private:

    void handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data) override;

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    ui::WidgetWindow* m_window = nullptr;

    ui::WidgetInputFloat3* m_propertyPosition = nullptr;
    ui::WidgetInputFloat3* m_propertyRotation = nullptr;
    ui::WidgetInputFloat3* m_propertyScale = nullptr;

    scene::DrawInstanceData* m_selectedObject = nullptr;
};
