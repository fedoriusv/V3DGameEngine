#include "EditorPropertyScreen.h"
#include "UI/Widgets.h"


EditorPropertyScreen::EditorPropertyScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_window(nullptr)

    , m_sceneData(nullptr)
    , m_selectedObject(nullptr)

    , m_loaded(false)
{
    m_propertyPosition.fill(nullptr);
    m_propertyRotation.fill(nullptr);
    m_propertyScale.fill(nullptr);
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

    if (m_selectedObject)
    {
        buildTransformProp();

        if (m_selectedObject->_type == scene::MaterialType::Lights)
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

    if (m_selectedObject)
    {
        const scene::Transform& transform = m_selectedObject->_transform;

        if (m_propertyPosition[0] && m_propertyPosition[1] && m_propertyPosition[2])
        {
            const math::Vector3D& pos = m_selectedObject->_transform.getPosition();
            m_propertyPosition[0]->setValue(pos.getX());
            m_propertyPosition[1]->setValue(pos.getY());
            m_propertyPosition[2]->setValue(pos.getZ());
        }

        if (m_propertyRotation[0] && m_propertyRotation[1] && m_propertyRotation[2])
        {
            const math::Vector3D& pos = m_selectedObject->_transform.getRotation();
            m_propertyRotation[0]->setValue(pos.getX());
            m_propertyRotation[1]->setValue(pos.getY());
            m_propertyRotation[2]->setValue(pos.getZ());
        }

        if (m_propertyScale[0] && m_propertyScale[1] && m_propertyScale[2])
        {
            const math::Vector3D& pos = m_selectedObject->_transform.getScale();
            m_propertyScale[0]->setValue(pos.getX());
            m_propertyScale[1]->setValue(pos.getY());
            m_propertyScale[2]->setValue(pos.getZ());
        }
    }
}

bool EditorPropertyScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_selectedObject = (selectionEvent->_selectedIndex != k_emptyIndex) ? &m_sceneData->m_generalList[selectionEvent->_selectedIndex]->_instance : nullptr;
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
    if (m_selectedObject)
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
                                this->m_propertyPosition[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D pos = m_selectedObject->_transform.getPosition();
                                    pos.setX(val);
                                    m_selectedObject->_transform.setPosition(pos);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyPosition[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D pos = m_selectedObject->_transform.getPosition();
                                    pos.setY(val);
                                    m_selectedObject->_transform.setPosition(pos);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyPosition[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D pos = m_selectedObject->_transform.getPosition();
                                    pos.setZ(val);
                                    m_selectedObject->_transform.setPosition(pos);
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
                                this->m_propertyRotation[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D rot = m_selectedObject->_transform.getRotation();
                                    rot.setX(val);
                                    m_selectedObject->_transform.setRotation(rot);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyRotation[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D rot = m_selectedObject->_transform.getRotation();
                                    rot.setY(val);
                                    m_selectedObject->_transform.setRotation(rot);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyRotation[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D rot = m_selectedObject->_transform.getRotation();
                                    rot.setZ(val);
                                    m_selectedObject->_transform.setRotation(rot);
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
                                this->m_propertyScale[0] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D scale = m_selectedObject->_transform.getScale();
                                    scale.setX(val);
                                    m_selectedObject->_transform.setScale(scale);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 1.f, 0.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyScale[1] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D scale = m_selectedObject->_transform.getScale();
                                    scale.setY(val);
                                    m_selectedObject->_transform.setScale(scale);
                                }
                            })
                    )
                    .addWidget(ui::WidgetInputDragFloat(0.f)
                        .setSize({ 100, 20 })
                        .setDragStep(0.1f)
                        .setBorderColor({ 0.f, 0.f, 1.f, 1.f })
                        .setOnCreated([this](ui::Widget* w) -> void
                            {
                                this->m_propertyScale[2] = static_cast<ui::WidgetInputDragFloat*>(w);
                            })
                        .setOnChangedValueEvent([this](ui::Widget* w, f32 val) -> void
                            {
                                if (this->m_selectedObject)
                                {
                                    math::Vector3D scale = m_selectedObject->_transform.getScale();
                                    scale.setZ(val);
                                    m_selectedObject->_transform.setScale(scale);
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
    if (m_selectedObject)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Light", flags)
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Intensity"))
                    .addWidget(ui::WidgetInputDragFloat(0.f))
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Temperature"))
                    .addWidget(ui::WidgetInputDragFloat(0.f))
                )
                .addWidget(ui::WidgetHorizontalLayout()
                    .addWidget(ui::WidgetText("Color"))
                    .addWidget(ui::WidgetInputDragFloat(0.f))
                )
            )
        );
}

void EditorPropertyScreen::buildMaterialProp()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed;
    if (m_selectedObject)
    {
        flags |= ui::WidgetTreeNode::TreeNodeFlag::Open;
    }

    window
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetTreeNode("Material", flags)
            )
        );
}
