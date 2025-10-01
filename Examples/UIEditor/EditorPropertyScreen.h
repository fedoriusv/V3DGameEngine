#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/Widgets.h"

#include "EditorScene.h"

using namespace v3d;

class EditorPropertyScreen final
{
public:

    EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorPropertyScreen();

    void registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData);

    void build();
    void update(f32 dt);

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    void buildTransformProp();
    void buildGeometryProp();
    void buildLightProp();
    void buildMaterialProp();

    ui::WidgetWindow* m_window;
    event::GameEventReceiver* m_gameEventRecevier;

    scene::SceneData* m_sceneData;
    scene::SceneNode* m_selectedNode;

    //transform
    struct TransformProperty
    {
        std::array<ui::WidgetInputDragFloat*, 3> m_position;
        std::array<ui::WidgetInputDragFloat*, 3> m_rotation;
        std::array<ui::WidgetInputDragFloat*, 3> m_scale;
        scene::TransformMode m_mode = scene::TransformMode::Local;
        u32 m_loadedFlag = 0;
    } m_transformProperty;

    //light
    struct Light
    {
        ui::WidgetInputDragFloat* m_propertyIntensity;
        ui::WidgetInputDragFloat* m_propertyTemperature;
        ui::WidgetColorPalette*   m_propertyColor;
        u32 m_loadedFlag = 0;
    } m_lightProperty;

    bool m_loaded;
};
