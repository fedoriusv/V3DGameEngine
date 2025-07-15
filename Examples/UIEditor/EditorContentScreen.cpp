#include "EditorContentScreen.h"

EditorContentScreen::EditorContentScreen() noexcept
    : m_loaded(false)
{
}

EditorContentScreen::~EditorContentScreen()
{
}

void EditorContentScreen::init(ui::WidgetWindow* widget)
{
    m_window = widget;
    m_loaded = false;
}

void EditorContentScreen::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    bool showSceneContent = true;
    if (showSceneContent)
    {
        window
            .addWidget(ui::WidgetLayout()
                .setFontSize(ui::WidgetLayout::MediumFont)
                .addWidget(ui::WidgetTreeNode("Scene", ui::WidgetTreeNode::TreeNodeFlag::Framed | ui::WidgetTreeNode::TreeNodeFlag::Open)
                    .addWidget(ui::WidgetText("test0"))
                    .addWidget(ui::WidgetText("test1"))
                    .addWidget(ui::WidgetText("test2"))
                    .addWidget(ui::WidgetTreeNode("node")
                        .addWidget(ui::WidgetText("test3"))
                    )
                )
            );
    }
}

void EditorContentScreen::update(f32 dt)
{
    if (!m_loaded)
    {
        build();
        m_loaded = true;
    }
}

void EditorContentScreen::handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data)
{
}

bool EditorContentScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorContentScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
