#include "EditorPropertyScreen.h"
#include "UI/Widgets.h"


EditorPropertyScreen::EditorPropertyScreen() noexcept
    : m_window(nullptr)
{
    m_propertyPosition.fill(nullptr);
    m_propertyRotation.fill(nullptr);
    m_propertyScale.fill(nullptr);
}

EditorPropertyScreen::~EditorPropertyScreen()
{
}

void EditorPropertyScreen::init(ui::WidgetWindow* widget)
{
    m_window = widget;

}

void EditorPropertyScreen::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    ui::WidgetTreeNode::TreeNodeFlags flags = ui::WidgetTreeNode::TreeNodeFlag::Framed | ui::WidgetTreeNode::TreeNodeFlag::Open;
    bool showTransformProps = true;
    if (showTransformProps)
    {
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

    bool showMaterialProps = true;
    if (showMaterialProps)
    {
        window
            .addWidget(ui::WidgetLayout()
                .setFontSize(ui::WidgetLayout::MediumFont)
                .addWidget(ui::WidgetTreeNode("Material", flags)
                )
            );
    }
}

void EditorPropertyScreen::update(f32 dt)
{
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

void EditorPropertyScreen::handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data)
{
    build();
    m_selectedObject = data.instanceObject;
}

bool EditorPropertyScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorPropertyScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
