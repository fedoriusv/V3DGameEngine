#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"

#include "EditorScene.h"
#include "UI/Widgets.h"

using namespace v3d;

class EditorContentScreen : public event::InputEventHandler, public utils::Observer<EditorReport>
{
public:

    EditorContentScreen() noexcept;
    ~EditorContentScreen();

    void init(ui::WidgetWindow* widget);
    void build();
    void update(f32 dt);

private:

    void handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data) override;

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

    ui::WidgetWindow* m_window = nullptr;

    ui::WidgetListBox* m_list;

    bool m_loaded;
};