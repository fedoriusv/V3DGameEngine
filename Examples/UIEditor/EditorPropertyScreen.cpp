#include "EditorPropertyScreen.h"
#include "UI/Widgets.h"


EditorPropertyScreen::EditorPropertyScreen() noexcept
    : m_window(nullptr)
{
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

    bool showTransformProps = true;
    if (showTransformProps)
    {
        window
            .addWidget(ui::WidgetLayout()
                .setFontSize(ui::WidgetLayout::MediumFont)
                .addWidget(ui::WidgetTreeNode("Transform")
                    .addWidget(ui::WidgetHorizontalLayout()
                        .addWidget(ui::WidgetText("Location"))
                        .addWidget(ui::WidgetInputFloat3(0.f, 0.f, 0.f)
                            .setOnCreated([this](ui::Widget* w) -> void
                                {
                                    this->m_propertyPosition = static_cast<ui::WidgetInputFloat3*>(w);
                                })
                            .setOnChangedValueEvent([this](ui::Widget* w, const math::float3& val) -> void
                                {
                                    if (this->m_selectedObject)
                                    {
                                        this->m_selectedObject->_transform.setPosition({ val._x, val._y, val._z });
                                    }
                                })
                        )
                    )
                    .addWidget(ui::WidgetHorizontalLayout()
                        .addWidget(ui::WidgetText("Rotation"))
                        .addWidget(ui::WidgetInputFloat3(0.f, 0.f, 0.f)
                            .setOnCreated([this](ui::Widget* w) -> void
                                {
                                    this->m_propertyRotation = static_cast<ui::WidgetInputFloat3*>(w);
                                })
                            .setOnChangedValueEvent([this](ui::Widget* w, const math::float3& val) -> void
                                {
                                    if (this->m_selectedObject)
                                    {
                                        this->m_selectedObject->_transform.setRotation({ val._x, val._y, val._z });
                                    }
                                })
                        )
                    )
                    .addWidget(ui::WidgetHorizontalLayout()
                        .addWidget(ui::WidgetText("Scale       "))
                        .addWidget(ui::WidgetInputFloat3(1.f, 1.f, 1.f)
                            .setOnCreated([this](ui::Widget* w) -> void
                                {
                                    this->m_propertyScale = static_cast<ui::WidgetInputFloat3*>(w);
                                })
                            .setOnChangedValueEvent([this](ui::Widget* w, const math::float3& val) -> void
                                {
                                    if (this->m_selectedObject)
                                    {
                                        this->m_selectedObject->_transform.setScale({ val._x, val._y, val._z });
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
                .addWidget(ui::WidgetTreeNode("Material")
                )
            );
    }
}

void EditorPropertyScreen::update(f32 dt)
{
    if (m_selectedObject)
    {
        const scene::Transform& transform = m_selectedObject->_transform;

        if (m_propertyPosition)
        {
            m_propertyPosition->setValue(transform.getPosition().getX(), transform.getPosition().getY(), transform.getPosition().getZ());
        }

        if (m_propertyRotation)
        {
            m_propertyRotation->setValue(transform.getRotation().getX(), transform.getRotation().getY(), transform.getRotation().getZ());
        }

        if (m_propertyScale)
        {
            m_propertyScale->setValue(transform.getScale().getX(), transform.getScale().getY(), transform.getScale().getZ());
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
