#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"

#include "EditorScene.h"
#include "UI/Widgets.h"

using namespace v3d;

class EditorAssetBrowser final
{
public:

    EditorAssetBrowser(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorAssetBrowser();

    void registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData);

    void build();
    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    ui::WidgetWindow* m_window;
    scene::SceneData* m_sceneData;

    bool m_loaded;
};