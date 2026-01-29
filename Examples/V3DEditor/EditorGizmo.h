#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Events/Input/InputEventHandler.h"
#include "Events/Game/GameEventHandler.h"
#include "UI/WidgetGizmo.h"

#include "EditorScene.h"

using namespace v3d;

class EditorGizmo final : public EditorScreen
{
public:

    EditorGizmo(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorGizmo();

    void registerWiget(ui::Widget* widget, scene::SceneData& sceneData) override;
    void show() override;
    void hide() override;

    void update(f32 dt) override;

    void modify(const math::Matrix4D& transform);
    void setEnable(bool enable);
    void setOperation(u32 index);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event);

private:

    ui::WidgetGizmo* m_gizmo;
    event::GameEventReceiver* m_gameEventRecevier;

    scene::SceneData* m_sceneData;
    scene::SceneNode* m_selectedNode;
    s32 m_currentOp;
};
