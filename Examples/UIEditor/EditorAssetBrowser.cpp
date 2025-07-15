#include "EditorAssetBrowser.h"

EditorAssetBrowser::EditorAssetBrowser() noexcept
    : m_loaded(false)
{
}

EditorAssetBrowser::~EditorAssetBrowser()
{
}

void EditorAssetBrowser::init(ui::WidgetWindow* widget)
{
    m_window = widget;
    m_loaded = false;
}

void EditorAssetBrowser::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    window.addWidget(ui::WidgetHorizontalLayout()
        .addWidget(ui::WidgetLayout(ui::WidgetLayout::Border)
                .setSize({200, 200})
                .setFontSize(ui::WidgetLayout::MediumFont)
                .addWidget(ui::WidgetText("test0"))
            //.addWidget(ui::WidgetTreeNode("Base")
            //    .addWidget(ui::WidgetText("test"))
            //)
            //.addWidget(ui::WidgetTreeNode("Project")
            //    .addWidget(ui::WidgetText("test"))
            //)
        )
        .addWidget(ui::WidgetLayout(ui::WidgetLayout::Border)
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetText("test1"))
        )
    );
}

void EditorAssetBrowser::update(f32 dt)
{
    if (!m_loaded)
    {
        build();
        m_loaded = true;
    }
}

void EditorAssetBrowser::handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data)
{
}

bool EditorAssetBrowser::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorAssetBrowser::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
