#include "EditorAssetBrowser.h"

EditorAssetBrowser::EditorAssetBrowser(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_window(nullptr)
    , m_sceneData(nullptr)

    , m_loaded(false)
{
}

EditorAssetBrowser::~EditorAssetBrowser()
{
}

void EditorAssetBrowser::registerWiget(ui::Widget* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_window = static_cast<ui::WidgetWindow*>(widget);
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorAssetBrowser::show()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(true);
}

void EditorAssetBrowser::hide()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(false);
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

bool EditorAssetBrowser::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::Custom && event->_customEventID == toEnumType(EditorEventType::SelectObject))
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_loaded = false;
    }

    return false;
}

bool EditorAssetBrowser::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
