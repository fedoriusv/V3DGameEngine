#include "EditorPropertyScreen.h"
#include "UI/Widgets.h"


EditorPropertyScreen::EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_window(nullptr)

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
    m_lightProperty.m_loadedFlag = 0;
}

EditorPropertyScreen::~EditorPropertyScreen()
{
}

void EditorPropertyScreen::registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_window = widget;
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorPropertyScreen::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    if (m_selectedNode)
    {
        buildTransformProp();

        if (m_selectedNode->_object->getType() == typeOf<scene::DirectionalLight>() || m_selectedNode->_object->getType() == typeOf<scene::PointLight>())
        {
            buildLightProp();
        }
        else
        {
            buildMaterialProp();
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
            const scene::Transform& transform = m_selectedNode->_instance._transform;

            ASSERT(m_transformProperty.m_position[0] && m_transformProperty.m_position[1] && m_transformProperty.m_position[2], "must be valid");
            const math::Vector3D& pos = transform.getPosition();
            m_transformProperty.m_position[0]->setValue(pos.getX());
            m_transformProperty.m_position[1]->setValue(pos.getY());
            m_transformProperty.m_position[2]->setValue(pos.getZ());

            ASSERT(m_transformProperty.m_rotation[0] && m_transformProperty.m_rotation[1] && m_transformProperty.m_rotation[2], "must be valid");
            const math::Vector3D& rot = transform.getRotation();
            m_transformProperty.m_rotation[0]->setValue(rot.getX());
            m_transformProperty.m_rotation[1]->setValue(rot.getY());
            m_transformProperty.m_rotation[2]->setValue(rot.getZ());

            ASSERT(m_transformProperty.m_scale[0] && m_transformProperty.m_scale[1] && m_transformProperty.m_scale[2], "must be valid");
            const math::Vector3D& scl = transform.getScale();
            m_transformProperty.m_scale[0]->setValue(scl.getX());
            m_transformProperty.m_scale[1]->setValue(scl.getY());
            m_transformProperty.m_scale[2]->setValue(scl.getZ());
        }

        if (m_lightProperty.m_loadedFlag & 0x7)
        {
            if (m_selectedNode->_object->getType() == typeOf<scene::DirectionalLight>())
            {
                f32 inten = static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->getIntensity();
                m_lightProperty.m_propertyIntensity->setValue(inten);

                f32 temp = static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->getTemperature();
                m_lightProperty.m_propertyTemperature->setValue(temp);

                color::ColorRGBF color = static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->getColor();
                m_lightProperty.m_propertyColor->setColor({ color._x, color._y, color._z, 1.f });
            }
        }
    }
}

bool EditorPropertyScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_selectedNode = (selectionEvent->_selectedIndex != k_emptyIndex) ? m_sceneData->m_generalList[selectionEvent->_selectedIndex] : nullptr;
        m_loaded = false;
    }

    return false;
}

bool EditorPropertyScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}

void EditorPropertyScreen::buildTransformProp()
{
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
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Location"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D pos = m_selectedNode->_instance._transform.getPosition();
                                    pos.setX(val);
                                    m_selectedNode->_instance._transform.setPosition(pos);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D pos = m_selectedNode->_instance._transform.getPosition();
                                    pos.setY(val);
                                    m_selectedNode->_instance._transform.setPosition(pos);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D pos = m_selectedNode->_instance._transform.getPosition();
                                    pos.setZ(val);
                                    m_selectedNode->_instance._transform.setPosition(pos);
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Rotation"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D rot = m_selectedNode->_instance._transform.getRotation();
                                    rot.setX(val);
                                    m_selectedNode->_instance._transform.setRotation(rot);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D rot = m_selectedNode->_instance._transform.getRotation();
                                    rot.setY(val);
                                    m_selectedNode->_instance._transform.setRotation(rot);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D rot = m_selectedNode->_instance._transform.getRotation();
                                    rot.setZ(val);
                                    m_selectedNode->_instance._transform.setRotation(rot);
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Scale       "))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D scale = m_selectedNode->_instance._transform.getScale();
                                    scale.setX(val);
                                    m_selectedNode->_instance._transform.setScale(scale);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D scale = m_selectedNode->_instance._transform.getScale();
                                    scale.setY(val);
                                    m_selectedNode->_instance._transform.setScale(scale);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
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
                                    math::Vector3D scale = m_selectedNode->_instance._transform.getScale();
                                    scale.setZ(val);
                                    m_selectedNode->_instance._transform.setScale(scale);
                                }
                            })
                    )
                )
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

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Light", flags)
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Intensity"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setDragStep(1.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyIntensity = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 0;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode && m_selectedNode->_object->getType() == typeOf<scene::DirectionalLight>())
                                {
                                    static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->setIntensity(val);
                                }
                            })
                    )
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Temperature"))
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setDragStep(10.f)
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                m_lightProperty.m_propertyTemperature = static_cast<ui::WidgetInputDragFloat*>(w);
                                m_lightProperty.m_loadedFlag |= 1 << 1;
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (m_selectedNode && m_selectedNode->_object->getType() == typeOf<scene::DirectionalLight>())
                                {
                                    static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->setTemperature(val);
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
                                m_lightProperty.m_loadedFlag |= 1 << 2;
                            })
                        .setOnColorChangedEvent([this](ui::Widget* w, const color::ColorRGBAF& color) -> void
                            {
                                if (m_selectedNode && m_selectedNode->_object->getType() == typeOf<scene::DirectionalLight>())
                                {
                                    static_cast<scene::DirectionalLight*>(m_selectedNode->_object)->setColor({ color._x, color._y, color._z });
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
