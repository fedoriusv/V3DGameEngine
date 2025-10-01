#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"

#include "EditorScene.h"
#include "UI/Widgets.h"

using namespace v3d;

class EditorContentScreen final
{
public:

    EditorContentScreen(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorContentScreen();

    void registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData);

    void build();
    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    event::GameEventReceiver* m_gameEventRecevier;
    ui::WidgetWindow* m_window;

    scene::SceneData* m_sceneData;
    ui::WidgetTreeNode* m_selectedWidget;
    std::map<scene::SceneNode*, ui::WidgetTreeNode*> m_widgetItems;


    bool m_loaded;
};