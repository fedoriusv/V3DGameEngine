#include "EditorPropertyScreen.h"
#include "UI/Widgets.h"


EditorPropertyScreen::EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_window(nullptr)
    , m_gameEventRecevier(gameEventRecevier)

    , m_sceneData(nullptr)
    , m_selectedNode(nullptr)

    , m_loaded(false)
{
    m_transformProperty.m_position.fill(nullptr);
    m_transformProperty.m_rotation.fill(nullptr);
    m_transformProperty.m_scale.fill(nullptr);
    m_transformProperty.m_loadedFlag = 0;

    m_lightProperty.m_propertyIntensity = nullptr;
    m_lightProperty.m_propertyTemperature = nullptr;
    m_lightProperty.m_propertyColor = nullptr;
    m_lightProperty.m_propertyRadius = nullptr;
    m_lightProperty.m_propertyAttenuation = nullptr;
    m_lightProperty.m_loadedFlag = 0;
}

EditorPropertyScreen::~EditorPropertyScreen()
{
}

void EditorPropertyScreen::registerWiget(ui::Widget* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_window = static_cast<ui::WidgetWindow*>(widget);
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorPropertyScreen::show()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(true);
}

void EditorPropertyScreen::hide()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(false);
}

void EditorPropertyScreen::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    m_transformProperty.m_loadedFlag = 0;
    m_lightProperty.m_loadedFlag = 0;

    if (m_selectedNode)
    {
        buildTransformProp();

        if (m_selectedNode->getComponentByType<scene::Mesh>())
        {
            buildGeometryProp();
            buildMaterialProp();
        }

        if (m_selectedNode->getComponentByType<scene::Light>())
        {
            buildLightProp();
        }
    }
}

void EditorPropertyScreen::update(f32 dt)
{
    if (!m_loaded)
    {
        build();
        m_loaded = true;
    }

    if (m_selectedNode)
    {
        if (m_transformProperty.m_loadedFlag & 0x1FF)
        {
            ASSERT(m_transformProperty.m_position[0] && m_transformProperty.m_position[1] && m_transformProperty.m_position[2], "must be valid");
            const math::Vector3D& pos = m_selectedNode->getTransform(m_transformProperty.m_mode).getPosition();
            m_transformProperty.m_position[0]->setValue(pos.getX());
            m_transformProperty.m_position[1]->setValue(pos.getY());
            m_transformProperty.m_position[2]->setValue(pos.getZ());

            ASSERT(m_transformProperty.m_rotation[0] && m_transformProperty.m_rotation[1] && m_transformProperty.m_rotation[2], "must be valid");
            const math::Vector3D& rot = m_selectedNode->getTransform(m_transformProperty.m_mode).getRotation();
            m_transformProperty.m_rotation[0]->setValue(rot.getX());
            m_transformProperty.m_rotation[1]->setValue(rot.getY());
            m_transformProperty.m_rotation[2]->setValue(rot.getZ());

            ASSERT(m_transformProperty.m_scale[0] && m_transformProperty.m_scale[1] && m_transformProperty.m_scale[2], "must be valid");
            const math::Vector3D& scl = m_selectedNode->getTransform(m_transformProperty.m_mode).getScale();
            m_transformProperty.m_scale[0]->setValue(scl.getX());
            m_transformProperty.m_scale[1]->setValue(scl.getY());
            m_transformProperty.m_scale[2]->setValue(scl.getZ());
        }

        if (m_lightProperty.m_loadedFlag & 0x31)
        {
            if (scene::DirectionalLight* light = m_selectedNode->getComponentByType<scene::DirectionalLight>(); light)
            {
                f32 inten = light->getIntensity();
                m_lightProperty.m_propertyIntensity->setValue(inten);

                f32 temp = light->getTemperature();
                m_lightProperty.m_propertyTemperature->setValue(temp);

                color::ColorRGBAF color = light->getColor();
                m_lightProperty.m_propertyColor->setColor(color);

                if (!m_selectedNode->m_children.empty())
                {
                    if (scene::Material* material = m_selectedNode->m_children.front()->getComponentByType<scene::Material>(); material)
                    {
                        material->setProperty<math::float4>("DiffuseColor", { color._x, color._y, color._z, 1.0 });
                    }
                }
            }
            else if (scene::PointLight* light = m_selectedNode->getComponentByType<scene::PointLight>(); light)
            {
                f32 inten = light->getIntensity();
                m_lightProperty.m_propertyIntensity->setValue(inten);

                f32 temp = light->getTemperature();
                m_lightProperty.m_propertyTemperature->setValue(temp);

                f32 radius = light->getRadius();
                m_lightProperty.m_propertyRadius->setValue(radius);

                color::ColorRGBAF color = light->getColor();
                m_lightProperty.m_propertyColor->setColor(color);

                math::float4 att = light->getAttenuation();
                m_lightProperty.m_propertyAttenuation->setValue(att._x, att._y, att._z, att._w);

                if (!m_selectedNode->m_children.empty())
                {
                    if (scene::Material* material = m_selectedNode->m_children.front()->getComponentByType<scene::Material>(); material)
                    {
                        material->setProperty<math::float4>("DiffuseColor", { color._x, color._y, color._z, 1.0 });
                    }
                }
            }

        }
    }
}

bool EditorPropertyScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_selectedNode = selectionEvent->_node;
        m_loaded = false;
    }
    else if (event->_eventType == event::GameEvent::GameEventType::TransformObject)
    {
    }

    return false;
}

bool EditorPropertyScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}

void EditorPropertyScreen::buildTransformProp()
{
    static f32 k_moveStep = 0.01f;
    static f32 k_rotStep = 0.1f;
    static f32 k_zoomStep = 0.01f;

    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed;
    if (m_selectedNode)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Transform", flags)
                .addWidget(ui::WidgetLayout()
                    .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
                    .addWidget(ui::WidgetRadioButtonGroup()
                        .addElement("local")
                        .addElement("global")
                        .setOnChangedIndexEvent([this](ui::Widget* w, u32 ind)
                            {
                                m_transformProperty.m_mode = scene::TransformMode(ind);
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Location"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_moveStep)
                        .setBorderColor({ 1.f, 0.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_position[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 0;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D pos = m_selectedNode->getTransform(m_transformProperty.m_mode).getPosition();
                                    pos.setX(val);
                                    m_selectedNode->setPosition(m_transformProperty.m_mode, pos);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_moveStep)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_position[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 1;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D pos = m_selectedNode->getTransform(m_transformProperty.m_mode).getPosition();
                                    pos.setY(val);
                                    m_selectedNode->setPosition(m_transformProperty.m_mode, pos);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_moveStep)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_position[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 2;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D pos = m_selectedNode->getTransform(m_transformProperty.m_mode).getPosition();
                                    pos.setZ(val);
                                    m_selectedNode->setPosition(m_transformProperty.m_mode, pos);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Rotation"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_rotStep)
                        .setBorderColor({ 1.f, 0.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_rotation[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 3;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D rot = m_selectedNode->getTransform(m_transformProperty.m_mode).getRotation();
                                    rot.setX(val);
                                    m_selectedNode->setRotation(m_transformProperty.m_mode, rot);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_rotStep)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_rotation[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 4;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D rot = m_selectedNode->getTransform(m_transformProperty.m_mode).getRotation();
                                    rot.setY(val);
                                    m_selectedNode->setRotation(m_transformProperty.m_mode, rot);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_rotStep)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_rotation[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 5;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D rot = m_selectedNode->getTransform(m_transformProperty.m_mode).getRotation();
                                    rot.setZ(val);
                                    m_selectedNode->setRotation(m_transformProperty.m_mode, rot);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Scale       "))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_zoomStep)
                        .setRange(0.01f, 1000.f)
                        .setBorderColor({ 1.f, 0.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_scale[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 6;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D scale = m_selectedNode->getTransform(m_transformProperty.m_mode).getScale();
                                    scale.setX(val);
                                    m_selectedNode->setScale(m_transformProperty.m_mode, scale);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_zoomStep)
                        .setRange(0.01f, 1000.f)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_scale[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 7;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D scale = m_selectedNode->getTransform(m_transformProperty.m_mode).getScale();
                                    scale.setY(val);
                                    m_selectedNode->setScale(m_transformProperty.m_mode, scale);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setStep(k_zoomStep)
                        .setRange(0.01f, 1000.f)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_transformProperty.m_scale[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_transformProperty.m_loadedFlag |= 1 << 8;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    math::Vector3D scale = m_selectedNode->getTransform(m_transformProperty.m_mode).getScale();
                                    scale.setZ(val);
                                    m_selectedNode->setScale(m_transformProperty.m_mode, scale);

                                    m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                }
                            })
                    )
                )
            )
        );
}

void EditorPropertyScreen::buildGeometryProp()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed;
    if (m_selectedNode)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    scene::Mesh* mesh = m_selectedNode->getComponentByType<scene::Mesh>();

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Geometry", flags)
                .addWidget(ui::WidgetText("Name: " + std::string(mesh->getName())))
                .addWidget(ui::WidgetText("Index Buffer: " + std::to_string(mesh->getIndexBuffer()->getIndicesCount()) + " indices"))
                .addWidget(ui::WidgetText("Vertex Buffer: " + std::to_string(mesh->getVertexBuffer(0)->getVerticesCount()) + " vertices"))
            )
        );
}

void EditorPropertyScreen::buildLightProp()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed;
    if (m_selectedNode)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    scene::Light* light = m_selectedNode->getComponentByType<scene::Light>();
    bool isPunctual = m_selectedNode->getComponentByType<scene::PointLight>() || m_selectedNode->getComponentByType<scene::SpotLight>();

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Light", flags)
                .addWidget(ui::WidgetText("Name: " + std::string(light->getName())))
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Intensity"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setStep(0.01f)
                        .setRange(-100.0f, 1000.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyIntensity = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 0;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    scene::Light* light = m_selectedNode->getComponentByType<scene::Light>();
                                    light->setIntensity(val);
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Temperature"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setStep(10.f)
                        .setRange(0.0f, 10000.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyTemperature = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 1;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    scene::Light* light = m_selectedNode->getComponentByType<scene::Light>();
                                    light->setTemperature(val);
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .setVisible(isPunctual)
                    .addWidget(ui::WidgetText("Radius"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setStep(0.01)
                        .setRange(0.01f, 1000.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyRadius = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 2;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    if (scene::PointLight* light = m_selectedNode->getComponentByType<scene::PointLight>(); light)
                                    {
                                        math::float4 attenuation = light->getAttenuation();
                                        light->setRadius(val);
                                        light->setAttenuation(attenuation._x, attenuation._y, attenuation._z, val);
                                        m_selectedNode->setScale(scene::TransformMode::Local, { val , val, val });

                                        m_gameEventRecevier->sendEvent(new EditorTransformEvent(m_selectedNode, m_transformProperty.m_mode, m_selectedNode->getTransform(m_transformProperty.m_mode)));
                                    }
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .setVisible(isPunctual)
                    .addWidget(ui::WidgetText("Attenuation"))
                    .addWidget(ui::WidgetInputDragFloat4(1.f, 0.09f, 0.063f, 0.f)
                    .setStep(0.01)
                    .setRange(0.01f, 100.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyAttenuation = static_cast<ui::WidgetInputDragFloat4*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 3;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, const math::float4& val) -> void
                            {
                                if (m_selectedNode)
                                {
                                    if (scene::PointLight* light = m_selectedNode->getComponentByType<scene::PointLight>(); light)
                                    {
                                        light->setAttenuation(val._x, val._y, val._z, val._w);

                                        light->setRadius(val._w);
                                        m_selectedNode->setScale(scene::TransformMode::Local, { val._w , val._w, val._w });
                                    }
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Color"))
                    .addWidget(ui::WidgetColorPalette()
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyColor = static_cast<ui::WidgetColorPalette*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 4;
                            })
                        .setOnColorChangedEvent([this](ui::Widget* w, const color::ColorRGBAF& color) -> void
                            {
                                if (m_selectedNode)
                                {
                                    scene::Light* light = m_selectedNode->getComponentByType<scene::Light>();
                                    light->setColor(color);
                                }
                            })
                    )
                )
            )
        );
}

void EditorPropertyScreen::buildMaterialProp()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;

    ObjectHandle dummy_h = m_sceneData->m_globalResources.get("default_white");
    ASSERT(dummy_h.isValid(), "must be valid");
    renderer::Texture2D* dummyTexture = objectFromHandle<renderer::Texture2D>(dummy_h);

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed;
    if (m_selectedNode)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Material", flags)
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("MaterialType"))
                    .addWidget(ui::WidgetText("<text>"))
                )
                .addWidget(ui::WidgetText("Name: <text>"))
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("BaseColor"))
                    .addWidget(ui::WidgetImage(dummyTexture, { 64, 64 }))
                )
                .addWidget(ui::WidgetText("Name: <text>"))
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Normal  "))
                    .addWidget(ui::WidgetImage(dummyTexture, { 64, 64 }))
                )
                .addWidget(ui::WidgetText("Name: <text>"))
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Material  "))
                    .addWidget(ui::WidgetImage(dummyTexture, { 64, 64 }))
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Tint"))
                    .addWidget(ui::WidgetColorPalette())
                )
            )
        );
}
