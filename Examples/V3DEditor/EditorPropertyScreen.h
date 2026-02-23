#pragma once

#include "Common.h"
#include "Events/Input/InputEventHandler.h"
#include "UI/Widgets.h"

#include "EditorScene.h"

using namespace v3d;

class EditorPropertyScreen final : public EditorScreen
{
public:

    EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorPropertyScreen();

    void registerWiget(ui::Widget* widget, scene::SceneData& sceneData) override;
    void show() override;
    void hide() override;

    void update(f32 dt) override;

public:

    bool handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event);
    bool handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event);

private:

    void build();
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
        ui::WidgetInputDragFloat*  m_propertyIntensity;
        ui::WidgetInputDragFloat*  m_propertyTemperature;
        ui::WidgetInputDragFloat*  m_propertyRadius;
        ui::WidgetInputDragFloat3* m_propertyAttenuation;
        ui::WidgetColorPalette*    m_propertyColor;
        ui::WidgetInputDragFloat2* m_propertySpotAngle;
        u32 m_loadedFlag = 0;
    } m_lightProperty;

    bool m_loaded;
};
